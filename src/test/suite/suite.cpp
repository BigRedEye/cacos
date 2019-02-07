#include "cacos/test/suite/suite.h"

#include "cacos/executable/executable.h"

#include "cacos/util/logger.h"

#include "cacos/util/mt/fixed_queue.h"

#include <termcolor/termcolor.hpp>

namespace cacos::test {

Suite::Suite(const config::Config& cfg)
    : workspace_(cfg.dir(config::DirType::test))
    , config_(cfg) {
    for (auto&& path : fs::recursive_directory_iterator(workspace_)) {
        if (path.path().filename() == Test::CONFIG_FILE) {
            Test test(workspace_, path.path());
            Type type = test.type();
            tests_[type].push_back(std::move(test));
        }
    }
}

struct RunResult {
    fs::path output;
    process::Result result;
    std::optional<process::Info> info;
    TestingResult diff;
};

/// TODO: refactor
void Suite::run(
    const RunOpts& opts,
    const executable::Executable& exe,
    util::optional_ref<const executable::Executable> checker) {

    static const std::string CHECKER_SUFFIX = "_check";

    executable::ExecPool pool(opts.limits);

    size_t doneTests = 0;
    size_t totalTests = 0;

    std::unordered_map<std::string, RunResult> context;

    util::mt::FixedQueue diffQueue;

    auto pushTest =
        [&](const executable::Executable& exe, const Test& test, bool isChecker = false) {
            if (!isChecker) {
                ++totalTests;
            }

            std::string name = test.name() + (isChecker ? CHECKER_SUFFIX : "");
            fs::path output = config_.dir(config::DirType::temp) / name;

            fs::create_directories(output.parent_path());

            context[name].output = output;

            auto callback = [&, isChecker, name = std::move(name)](
                                process::Result res, std::optional<process::Info>&& info) {
                auto& ctx = context[name];
                ctx.result = res;
                ctx.info = std::move(info);

                Logger::log().print(
                    "Checker = {}, exit status = {}, return code = {}, cpu time = {:.3f} s, max "
                    "rss = {:.3f} mb",
                    isChecker,
                    process::status::serialize(res.status),
                    res.returnCode,
                    ctx.info->cpuTime.count(),
                    ctx.info->maxRss / (1024. * 1024.));

                if (!isChecker) {
                    ++doneTests;
                    Logger::log().print(
                        "Done {} / {} ({:.1f}%)\n",
                        doneTests,
                        totalTests,
                        doneTests * 100. / totalTests);
                }
            };

            TaskContext taskContext{exe, std::move(callback), output};

            pool.push(test.task(std::move(taskContext)));
        };

    auto success = [&](const Test& test, RunResult& result) -> bool {
        return result.result.status == process::status::OK && result.result.returnCode == test.returnCode();
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
        Logger::warning().print(
            "No checker specified; ignoring {} tests", tests_[Type::diff].size());
    }

    pool.run();

    Logger::log().print("Computing diffs");

    diffQueue.run();
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

            if (checker && !success(test, context[test.name() + CHECKER_SUFFIX])) {
                ++crashed;

                std::cout << termcolor::red;
                std::string status = process::status::serialize(context[test.name() + CHECKER_SUFFIX].result.status);
                fmt::print(std::cout, " Checker failure: {}\n", status);
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
                fmt::print(std::cout, ", cpu time: {:.3f} seconds, max rss: {:.3f} MiB",
                   ctx.info->cpuTime.count(),
                   ctx.info->maxRss / (1024. * 1024.)
               );
            }
            std::cout << std::endl << termcolor::reset;

            if (ctx.result.status != process::status::WA) {
                continue;
            }

            fmt::print(std::cout, "Diff:\n");

            std::visit([](auto&& value) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, util::diff::Unified>) {
                    value.print(std::cout);
                } else if constexpr (std::is_same_v<T, BlobDiff>) {
                    std::cout << termcolor::red << value.left() << termcolor::reset << std::endl;
                    std::cout << termcolor::green << value.right() << termcolor::reset << std::endl;
                }
            }, ctx.diff.diff);
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
    fmt::print(std::cout, "[Passed      ]: {} / {} tests ({:.1f}%)\n", passed, total, passed * 100. / total);
    fmt::print(std::cout, "[Crashed     ]: {} / {} tests ({:.1f}%)\n", crashed, total, crashed * 100. / total);
    fmt::print(std::cout, "[Wrong answer]: {} / {} tests ({:.1f}%)\n", failed, total, failed * 100. / total);

    std::cout << termcolor::reset;
}

} // namespace cacos::test
