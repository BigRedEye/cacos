#include "cacos/test/generate/generator.h"

#include "cacos/executable/executable.h"

#include "cacos/lang/lang.h"

#include "cacos/test/suite/test.h"

#include "cacos/util/logger.h"
#include "cacos/util/mt/fixed_queue.h"
#include "cacos/util/progress_bar.h"
#include "cacos/util/string.h"

#include <boost/asio.hpp>

#include <boost/container/stable_vector.hpp>

#include <functional>

namespace cacos::test {

Generator::Generator(const config::Config& cfg, const GeneratorOptions& opts)
    : config_(cfg)
    , opts_(opts) {
}

void Generator::run() {
    executable::Executable exe = config_.langs().runnable(
        {opts_.generatorSources, {opts::hostArch(), opts::BuildType::release}});

    InlineVariables vars;

    executable::ExecPool pool(
        process::Limits{process::Limits::unlimited<bytes>, seconds(1.0), seconds(2.0)},
        opts_.threads);

    boost::container::stable_vector<std::string> stdIn;
    boost::container::stable_vector<fs::path> stdOut;
    boost::container::stable_vector<fs::path> stdErr;
    std::vector<std::pair<Test, fs::path>> generatedTests;

    size_t totalTasks = 0;
    size_t crashedTasks = 0;

    util::ProgressBar<size_t> bar;
    traverse(opts_.vars.begin(), vars, [&](const InlineVariables& vars) {
        ++totalTasks;

        std::string name = vars.parse(opts_.name);
        fs::path dir = config_.dir(config::DirType::cache) / name;

        if (!fs::exists(dir)) {
            fs::create_directories(dir);
        }

        stdIn.emplace_back(vars.parse(opts_.genIO.input));
        stdOut.emplace_back(dir / "gen.stdout");
        stdErr.emplace_back(dir / "gen.stderr");
        log::log().print("{}", stdIn.back());

        std::vector<std::string> testArgs;
        testArgs.reserve(opts_.testArgs.size());
        for (auto&& arg : opts_.testArgs) {
            testArgs.push_back(vars.parse(arg));
        }

        auto callback = [&, dir, name, testArgs, i = totalTasks - 1](
                            process::Result res, std::optional<process::Info>&& info) {
            if (res.status != process::status::OK || res.returnCode != 0) {
                ++crashedTasks;
            } else {
                test::Test test;
                test.name(name).type(opts_.type).args(testArgs).input(dir / opts_.testIO.input);
                if (opts_.type == Type::canonical) {
                    test.output(dir / opts_.testIO.output);
                }

                generatedTests.emplace_back(std::move(test), std::move(dir));
            }

            log::log().print(
                "Run {} / {}: Exit status: {}, return code = {}, cpu time = {:.3f} s, max rss = "
                "{:.3f} mb",
                i,
                totalTasks,
                process::status::serialize(res.status),
                res.returnCode,
                info->cpuTime.count(),
                info->maxRss / (1024. * 1024.));

            bar.process(1);
        };

        std::vector<std::string> args;
        args.reserve(opts_.args.size());
        for (auto&& s : opts_.args) {
            args.push_back(vars.parse(s));
        }

        executable::ExecTaskContext ctx{args, boost::this_process::environment(), dir, callback};
        auto task = executable::makeTask(
            exe,
            std::move(ctx),
            bp::buffer(std::as_const(stdIn.back())),
            stdOut.back().string(),
            stdErr.back().string());

        pool.push(std::move(task));
    });

    bar.reset(totalTasks);

    pool.run();

    util::mt::FixedQueue serializationPool;
    for (auto& p : generatedTests) {
        serializationPool.add([&p, this] {
            auto& [test, dir] = p;
            test.serialize(config_.dir(config::DirType::test), opts_.force);
            fs::remove_all(dir);
        });
    }

    serializationPool.run();

    fmt::print(std::cout, "Generated {} / {} tests\n", totalTasks - crashedTasks, totalTasks);
    if (crashedTasks > 0) {
        fmt::print(std::cout, "Generator crashed in {} / {} tests\n", crashedTasks, totalTasks);
        fmt::print(
            std::cout,
            "Working directories of crashed tests: {}.\nUse --keep-working-dirs to save them\n",
            config_.dir(config::DirType::cache));
    }
}

void Generator::traverse(
    VarsIterator it,
    InlineVariables& vars,
    const std::function<void(const InlineVariables& vars)>& callback) {
    if (it == opts_.vars.end()) {
        callback(vars);
    } else {
        auto next = std::next(it);
        for (i64 i = it->second.from; i < it->second.to; i += it->second.step) {
            vars.set(it->first, util::string::to(i));
            traverse(next, vars, callback);
        }
    }
}

} // namespace cacos::test
