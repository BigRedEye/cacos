#include "cacos/test/generate/generator.h"

#include "cacos/executable/executable.h"

#include "cacos/util/logger.h"
#include "cacos/util/string.h"

#include "cacos/lang/lang.h"

#include <boost/asio.hpp>

#include <boost/container/stable_vector.hpp>

#include <functional>

namespace cacos::test {

Generator::Generator(const config::Config& cfg, const GeneratorOptions& opts)
    : config_(cfg)
    , opts_(opts) {
}

void Generator::run() {
    executable::Executable exe =
        config_.langs().runnable(fs::current_path() / opts_.generator);

    InlineVariables vars;

    executable::Flags flags(opts_.args);
    executable::ExecPool pool(process::Limits{
        process::Limits::unlimited<bytes>,
        seconds(0.3),
        seconds(1.0)
    });

    boost::container::stable_vector<std::string> input;
    boost::container::stable_vector<fs::path> output;

    size_t doneTasks = 0;
    size_t totalTasks = 0;

    traverse(opts_.vars.begin(), vars, [&](const InlineVariables& vars) {
        ++totalTasks;

        auto res = flags.build(vars);
        input.emplace_back(vars.parse(opts_.input));
        output.emplace_back(
            config_.dir(config::DirType::test) / vars.parse(opts_.testName));

        auto callback = [&] (process::Result res, std::optional<process::Info>&& info) {
            if (res.status != process::status::OK) {
                Logger::warning().print("Exit status: {}", process::status::serialize(res.status));
            }

            Logger::log().print(
                "Return code = {}, cpu time = {:.3f} s, max rss = {:.3f} mb",
                res.returnCode,
                info->cpuTime.count(),
                info->maxRss / (1024. * 1024.)
            );

            ++doneTasks;
            Logger::info().print("Done {} / {} ({:.1f}%)", doneTasks, totalTasks, doneTasks * 100. / totalTasks);
        };

        std::vector<std::string> args;
        args.reserve(opts_.args.size());
        for (auto&& s : opts_.args) {
            args.push_back(vars.parse(s));
        }

        executable::ExecTaskContext ctx {
            args, boost::this_process::environment(), callback
        };
        auto task = executable::makeTask(
            exe,
            std::move(ctx),
            bp::buffer(std::as_const(input.back())),
            output.back().string(),
            bp::null
        );

        pool.push(std::move(task));
    });

    pool.run();
}

void Generator::traverse(VarsIterator it, InlineVariables& vars, const std::function<void(const InlineVariables& vars)>& callback) {
    if (it == opts_.vars.end()) {
        callback(vars);
    } else {
        auto next = std::next(it);
        for (i64 i = it->second.from; i < it->second.to; i += it->second.step) {
            vars.set(it->first, util::to_string(i));
            traverse(next, vars, callback);
        }
    }
}

} // namespace cacos::test
