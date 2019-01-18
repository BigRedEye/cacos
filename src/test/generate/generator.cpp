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
        config_.langs().runnable(opts_.workspace / opts_.generator);

    InlineVariables vars;

    executable::Flags flags(opts_.args);
    executable::ExecPool pool(process::Limits{
        process::Limits::unlimited<bytes>,
        seconds(0.3)
    });

    boost::container::stable_vector<std::string> input;
    boost::container::stable_vector<std::future<std::string>> output;
    traverse(opts_.vars.begin(), vars, [&](const InlineVariables& vars) {
        auto res = flags.build(vars);
        input.emplace_back(vars.parse(opts_.input));
        output.emplace_back();

        auto callback = [] (process::Result res, std::optional<process::Info>&& info) {
            if (res.status != process::status::OK) {
                Logger::warning() << "Exit status: " << process::status::serialize(res.status);
            }
            Logger::log() << "Return code = "
                << res.returnCode
                << ", cpu time = " << info->cpuTime.count()
                << ", real time = " << info->realTime.count()
                << ", max rss = " << info->maxRss / (1024. * 1024.) << "mb";
        };
        auto task = executable::makeTask(
            exe,
            callback,
            res,
            bp::buffer(std::as_const(input.back())),
            std::ref(output.back()),
            bp::null
        );

        pool.push(std::move(task));
    });

    pool.run();

    for (size_t i = 0; i < input.size(); ++i) {
        Logger::info() << input[i] << ": " << output[i].get();
    }
}

void Generator::traverse(VarsIterator it, InlineVariables& vars, const std::function<void(const InlineVariables& vars)>& callback) {
    if (it == opts_.vars.end()) {
        callback(vars);
    } else {
        auto next = std::next(it);
        for (int i = it->second.from; i < it->second.to; i += it->second.step) {
            vars.set(it->first, util::to_string(i));
            traverse(next, vars, callback);
        }
    }
}

} // namespace cacos::test
