#include "cacos/test/run/run.h"
#include "cacos/test/suite/suite.h"

#include "cacos/config/config.h"

#include <cpparg/cpparg.h>

namespace cacos::test {

int run(int argc, const char* argv[]) {
    cpparg::parser parser("cacos test run");
    parser.title("Run tests");

    config::Config cfg(parser, config::LANGS | config::TASK_EXE);

    std::optional<std::vector<fs::path>> sources;

    parser.add("exe")
        .optional()
        .value_type("SOURCES")
        .description("Source files, separated with commas or spaces [default = from config]")
        .handle([&](auto sv) {
            auto splitted = util::split(sv, ", ");
            sources = std::vector<fs::path>{};
            for (auto&& path : splitted) {
                if (fs::exists(path)) {
                    sources->emplace_back(path);
                } else {
                    throw std::runtime_error("Cannot find source file " + util::str(path));
                }
            }
        });

    std::optional<std::vector<fs::path>> checkerSources;
    parser.add("checker")
        .optional()
        .value_type("SOURCES")
        .description("Checker source files, separated with commas or spaces")
        .handle([&](auto sv) {
            auto splitted = util::split(sv, ", ");
            checkerSources = std::vector<fs::path>{};
            for (auto&& path : splitted) {
                fs::path p{path};
                if (!p.is_absolute()) {
                    p = fs::absolute(path);
                }
                if (fs::exists(p)) {
                    checkerSources->emplace_back(p);
                }
            }
        });

    std::optional<std::vector<fs::path>> diffSources;
    parser.add("diff")
        .optional()
        .value_type("SOURCES")
        .description("Diff tool source files / executable, separated with commas or spaces")
        .handle([&](auto sv) {
            auto splitted = util::split(sv, ", ");
            diffSources = std::vector<fs::path>{};
            for (auto&& path : splitted) {
                fs::path p{path};
                if (!p.is_absolute()) {
                    p = fs::absolute(path);
                }
                if (fs::exists(p)) {
                    diffSources->emplace_back(p);
                }
            }
        });

    std::string prefix;
    parser.add("suite")
        .optional()
        .value_type("SUITE")
        .description("Test name to run (name prefix) [default = \"\"]")
        .store(prefix);

    RunOpts runOpts{
        cfg.task().limits
    };

    parser.add("stats").optional().no_argument().description("Print run stats").handle([&](auto) {
        runOpts.printInfo = true;
    });

    parser.add("threads")
        .optional()
        .description("Process limit")
        .default_value(util::string::to(std::thread::hardware_concurrency()))
        .handle<size_t>([&](size_t workers) { runOpts.workers = workers; });

    parser.add("tl")
        .optional()
        .value_type("SECONDS")
        .description("Time limit")
        .handle<double>([&](double s) {
            runOpts.limits.cpu = seconds(s);
            runOpts.limits.real = seconds(s);
        });

    parser.add("ml")
        .optional()
        .value_type("MiB")
        .description("Memory limit")
        .handle<double>(
            [&](double ram) { runOpts.limits.ml = static_cast<bytes>(ram * 1024. * 1024.); });

    parser.add('z', "allow-nonzero-return-codes")
        .optional()
        .description("Allow nonzero return codes")
        .no_argument()
        .handle([&] { runOpts.allowNonZeroReturnCodes = true; });

    parser.parse(argc, argv);

    cfg.ensureWorkspaceExistence();

    if (!sources) {
        sources = cfg.task().exe.sources;
    }

    opts::CompilerOpts compilerOpts = cfg.task().exe.compiler;
    executable::Executable main = cfg.langs().runnable({sources.value(), compilerOpts});

    test::Suite suite(cfg, prefix);

    std::optional<executable::Executable> diff;
    if (diffSources) {
        diff = cfg.langs().runnable({diffSources.value(), compilerOpts});
    }

    std::optional<executable::Executable> checker;
    if (checkerSources) {
        checker = cfg.langs().runnable({checkerSources.value(), compilerOpts});
    }
    suite.run(runOpts, main, diff, checker);

    return 0;
}

} // namespace cacos::test
