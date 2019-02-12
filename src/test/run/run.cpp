#include "cacos/test/run/run.h"
#include "cacos/test/suite/suite.h"

#include "cacos/config/config.h"

#include <cpparg/cpparg.h>

namespace cacos::test {

int run(int argc, const char* argv[]) {
    cpparg::parser parser("cacos test run");
    parser.title("Run tests");

    config::Config cfg(parser, config::LANGS);

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
                if (fs::exists(path)) {
                    checkerSources->emplace_back(path);
                }
            }
        });

    std::string name;
    parser.add("test")
        .optional()
        .value_type("Test name")
        .description("Test name to run [default = ALL]")
        .store(name);

    RunOpts runOpts;
    parser.add("info").optional().no_argument().description("Print run stats").handle([&](auto) {
        runOpts.printInfo = true;
    });

    parser.add("tl")
        .optional()
        .value_type("SECONDS")
        .description("Time limit")
        .default_value(1.0)
        .handle<double>([&](double s) {
            runOpts.limits.cpu = seconds(s);
            runOpts.limits.real = seconds(s);
        });

    parser.add("ml")
        .optional()
        .value_type("MiB")
        .description("Memory limit")
        .default_value(64.0)
        .handle<double>(
            [&](double ram) { runOpts.limits.ml = static_cast<bytes>(ram * 1024. * 1024.); });

    parser.parse(argc, argv);

    if (!sources) {
        sources = cfg.task().exe.sources;
    }

    opts::CompilerOpts compilerOpts = cfg.task().exe.compiler;
    executable::Executable main = cfg.langs().runnable({sources.value(), compilerOpts});

    test::Suite suite(cfg);

    if (checkerSources) {
        executable::Executable checker =
            cfg.langs().runnable({checkerSources.value(), compilerOpts});
        suite.run(runOpts, main, checker);
    } else {
        suite.run(runOpts, main);
    }

    return 0;
}

} // namespace cacos::test