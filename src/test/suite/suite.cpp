#include "cacos/test/suite/suite.h"
#include "cacos/executable/executable.h"
#include "cacos/util/logger.h"
#include "cacos/util/mt/fixed_queue.h"
#include "cacos/util/progress_bar.h"

#include <boost/container/stable_vector.hpp>
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

class DiffComputer {
public:
    DiffComputer(bool hasExternalDiff) {
        if (hasExternalDiff) {
            workers_.emplace<executable::ExecPool>();
        } else {
            workers_.emplace<util::mt::FixedQueue>();
        }
    }

    void push(
        const Test& test,
        const std::optional<const executable::Executable>& diff,
        TestingResult& result,
        fs::path out,
        std::optional<fs::path> expected = std::nullopt) {
        if (diff) {
            // TODO: struct CompareResult { stdout, stderr, exitcode };
            stdouts_.emplace_back();
            stderrs_.emplace_back();
            exitCodes_.emplace_back();
            results_.emplace_back(result);

            auto task = test.compareExternal(
                diff.value(), out, expected, exitCodes_.back(), stdouts_.back(), stderrs_.back());
            std::get<executable::ExecPool>(workers_).push(std::move(task));
        } else {
            std::get<util::mt::FixedQueue>(workers_).add(
                [&result, &test, out, expected] { result = test.compare(out, expected); });
        }
    }

    void run() {
        std::visit([](auto&& value) { value.run(); }, workers_);

        collect();
    }

private:
    void collect() {
        for (size_t i = 0; i < results_.size(); ++i) {
            std::string stdOut = stdouts_[i].get();
            std::string stdErr = stderrs_[i].get();
            if (exitCodes_[i] != 0 || !stdOut.empty() || !stdErr.empty()) {
                results_[i].get().diff.emplace<ExternalDiff>(
                    ExternalDiff{exitCodes_[i], std::move(stdOut), std::move(stdErr)});
            } else {
                results_[i].get().diff.emplace<NoDiff>();
            }
        }
    }

private:
    std::variant<executable::ExecPool, util::mt::FixedQueue> workers_;

    boost::container::stable_vector<std::future<std::string>> stdouts_;
    boost::container::stable_vector<std::future<std::string>> stderrs_;
    boost::container::stable_vector<int> exitCodes_;
    std::vector<std::reference_wrapper<TestingResult>> results_;
};

void Suite::run(
    const RunOpts& opts,
    const executable::Executable& exe,
    const std::optional<const executable::Executable>& diff,
    const std::optional<const executable::Executable>& checker) {
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

                if (res.status != process::status::OK ||
                    (res.returnCode != 0 && !opts.allowNonZeroReturnCodes)) {
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

    for (auto&& test : tests_[Type::canonical]) {
        pushTest(exe, test);
    }

    if (checker) {
        for (auto&& test : tests_[Type::diff]) {
            pushTest(exe, test, false);
            pushTest(checker.value(), test, true);
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

    DiffComputer diffs{diff.has_value()};

    auto pushComputeDiff = [&](const Test& test) {
        auto& ctx = context[test.name()];
        if (ctx.result.status == process::status::OK) {
            if (test.type() == Type::canonical) {
                diffs.push(test, diff, ctx.diff, ctx.output);
            } else {
                auto checker = context.at(test.name() + CHECKER_SUFFIX);
                if (success(test, checker)) {
                    diffs.push(
                        test,
                        diff,
                        ctx.diff,
                        ctx.output,
                        context.at(test.name() + CHECKER_SUFFIX).output);
                }
            }
        }
    };

    for (auto&& test : tests_[Type::canonical]) {
        pushComputeDiff(test);
    }
    if (checker) {
        for (auto&& test : tests_[Type::diff]) {
            pushComputeDiff(test);
        }
    }

    diffs.run();

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
                    } else if constexpr (std::is_same_v<T, ExternalDiff>) {
                        if (value.exitCode != 0) {
                            std::cout << termcolor::red << "diff exit code: " << termcolor::reset
                                      << value.exitCode << std::endl;
                        }
                        if (!value.stdOut.empty()) {
                            std::cout << termcolor::red << "diff stdout: " << termcolor::reset
                                      << value.stdOut << std::endl;
                        }
                        if (!value.stdErr.empty()) {
                            std::cout << termcolor::red << "diff stderr: " << termcolor::reset
                                      << value.stdErr << std::endl;
                        }
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
