#include "cacos/test/suite/suite.h"

#include "cacos/executable/executable.h"

#include "cacos/util/logger.h"

#include "cacos/util/mt/fixed_queue.h"

#include "cacos/util/progress_bar.h"

#include <termcolor/termcolor.hpp>

/**
 * TODO: refactor
 */

namespace cacos::test {

Suite::Suite(const config::Config& cfg, std::string_view prefix)
    : workspace_(cfg.dir(config::DirType::test))
    , config_(cfg) {
    fs::path searchRoot = workspace_;
    if (!prefix.empty()) {
        searchRoot /= prefix;
    }

    if (!fs::exists(searchRoot)) {
        throw std::runtime_error(util::string::join("Cannot find test suite ", prefix));
    }

    for (auto&& path : fs::recursive_directory_iterator(searchRoot)) {
        if (path.path().filename() == Test::CONFIG_FILE) {
            Test test(workspace_, path.path());
            Type type = test.type();
            tests_[type].push_back(std::move(test));
        }
    }

    /* sort tests by name */
    for (auto&& [k, v] : tests_) {
        std::sort(v.begin(), v.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.name() < rhs.name();
        });
    }
}

struct RunResult {
    fs::path output;
    process::Result result;
    std::optional<process::Info> info;
    TestingResult diff;
};

void Suite::run(
    const RunOpts& opts,
    const executable::Executable& exe,
    util::optional_ref<const executable::Executable> checker) {
    static const std::string CHECKER_SUFFIX = "_check";

    executable::ExecPool pool(opts.limits, opts.workers);

    size_t doneTests = 0;
    size_t totalTests = 0;
    size_t totalTasks = 0;
    size_t crashedRuns = 0;

    std::unordered_map<std::string, RunResult> context;

    util::mt::FixedQueue diffQueue;

    util::ProgressBar<size_t> bar;

    auto pushTest =
        [&](const executable::Executable& exe, const Test& test, bool isChecker = false) {
            if (!isChecker) {
                ++totalTests;
            }
            ++totalTasks;

            std::string name = test.name() + (isChecker ? CHECKER_SUFFIX : "");
            fs::path output = config_.dir(config::DirType::cache) / name / "test.stdout";

            fs::create_directories(output.parent_path());

            context[name].output = output;

            auto callback = [&, isChecker, name = std::move(name)](
                                process::Result res, std::optional<process::Info>&& info) {
                auto& ctx = context[name];
                ctx.result = res;
                ctx.info = std::move(info);

                log::log().print(
                    "Checker = {}, exit status = {}, return code = {}, cpu time = {:.3f} s, max "
                    "rss = {:.3f} mb",
                    isChecker,
                    process::status::serialize(res.status),
                    res.returnCode,
                    ctx.info->cpuTime.count(),
                    ctx.info->maxRss / (1024. * 1024.));

                if (!isChecker) {
                    ++doneTests;
                    log::log().print(
                        "Done {} / {} ({:.1f}%)\n",
                        doneTests,
                        totalTests,
                        doneTests * 100. / totalTests);
                }

                if (res.status != process::status::OK || (res.returnCode != 0 && !opts.allowNonZeroReturnCodes)) {
                    ++crashedRuns;
                }

                bar.process(1, fmt::format("Crashed runs: {}", crashedRuns));
            };

            TaskContext taskContext{exe, std::move(callback), output};

            pool.push(test.task(std::move(taskContext)));
        };

    auto success = [&](const Test& test, RunResult& result) -> bool {
        return result.result.status == process::status::OK &&
               result.result.returnCode == test.returnCode();
    };

    auto pushComputeDiff = [&](const Test& test) {
        diffQueue.add([&] {
            auto& ctx = context[test.name()];
            if (ctx.result.status == process::status::OK) {
                if (test.type() == Type::canonical) {
                    ctx.diff = test.compare(ctx.output);
                } else {
                    auto checker = context.at(test.name() + CHECKER_SUFFIX);
                    if (success(test, checker)) {
                        ctx.diff = test.compare(
                            ctx.output, context.at(test.name() + CHECKER_SUFFIX).output);
                    }
                }
            }
        });
    };

    for (auto&& test : tests_[Type::canonical]) {
        pushTest(exe, test);
        pushComputeDiff(test);
    }

    if (checker) {
        for (auto&& test : tests_[Type::diff]) {
            pushTest(exe, test, false);
            pushTest(checker.value(), test, true);
            pushComputeDiff(test);
        }
    } else if (!tests_[Type::diff].empty()) {
        log::warning().print("No checker specified; ignoring {} tests", tests_[Type::diff].size());
    }

    if (totalTests == 0) {
        log::warning().print("No tests found");
        return;
    }

    bar.reset(totalTasks);

    pool.run();

    log::log().print("Computing diffs");

    diffQueue.start();
    diffQueue.wait();

    ui64 crashed = 0;
    ui64 failed = 0;
    ui64 passed = 0;

    auto processResults = [&](Type type) {
        size_t namesWidth = 0;
        for (auto&& test : tests_[type]) {
            namesWidth = std::max(namesWidth, test.name().size());
        }

        for (auto&& test : tests_[type]) {
            fmt::print(std::cout, "[{:<{}}]", test.name(), namesWidth);

            auto& checkerCtx = context[test.name() + CHECKER_SUFFIX];
            if (checker && !success(test, checkerCtx)) {
                ++crashed;

                std::cout << termcolor::red;
                std::string status = process::status::serialize(checkerCtx.result.status);
                fmt::print(
                    std::cout,
                    " Checker failure: {}, exit code = {}\n",
                    status,
                    checkerCtx.result.returnCode);
                std::cout << termcolor::reset;
                continue;
            }

            auto& ctx = context[test.name()];

            if (ctx.result.returnCode != test.returnCode() &&
                ctx.result.status == process::status::OK) {
                ctx.result.status = process::status::RE;
            }

            if (!success(test, ctx)) {
                ++crashed;

                std::cout << termcolor::red;
            } else if (!std::holds_alternative<NoDiff>(ctx.diff.diff)) {
                ++failed;

                ctx.result.status = process::status::WA;
                std::cout << termcolor::yellow;
            } else {
                ++passed;

                std::cout << termcolor::green;
            }

            fmt::print(std::cout, " Result: {}", process::status::serialize(ctx.result.status));
            if (opts.printInfo) {
                fmt::print(
                    std::cout,
                    ", cpu time: {:.3f} seconds, max rss: {:.3f} MiB",
                    ctx.info->cpuTime.count(),
                    ctx.info->maxRss / (1024. * 1024.));
            }
            std::cout << std::endl << termcolor::reset;

            if (ctx.result.status != process::status::WA) {
                continue;
            }

            fmt::print(std::cout, "Diff:\n");

            std::visit(
                [](auto&& value) {
                    using T = std::decay_t<decltype(value)>;
                    if constexpr (std::is_same_v<T, util::diff::Unified>) {
                        value.print(std::cout);
                    } else if constexpr (std::is_same_v<T, BlobDiff>) {
                        std::cout << termcolor::red << value.left() << termcolor::reset
                                  << std::endl;
                        std::cout << termcolor::green << value.right() << termcolor::reset
                                  << std::endl;
                    }
                },
                ctx.diff.diff);
        }
    };

    processResults(Type::canonical);
    if (checker) {
        processResults(Type::diff);
    }

    std::cout << termcolor::bold;
    if (crashed > 0) {
        std::cout << termcolor::red;
    } else if (failed > 0) {
        std::cout << termcolor::yellow;
    } else {
        std::cout << termcolor::green;
    }

    ui64 total = crashed + failed + passed;

    fmt::print(std::cout, "\n");
    fmt::print(
        std::cout,
        "[Passed      ]: {} / {} tests ({:.1f}%)\n",
        passed,
        total,
        passed * 100. / total);
    fmt::print(
        std::cout,
        "[Wrong answer]: {} / {} tests ({:.1f}%)\n",
        failed,
        total,
        failed * 100. / total);
    fmt::print(
        std::cout,
        "[Crashed     ]: {} / {} tests ({:.1f}%)\n",
        crashed,
        total,
        crashed * 100. / total);

    std::cout << termcolor::reset;
}

} // namespace cacos::test
