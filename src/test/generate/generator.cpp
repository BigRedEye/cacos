#include "cacos/test/generate/generator.h"

#include "cacos/executable/executable.h"

#include "cacos/util/logger.h"
#include "cacos/util/string.h"

#include <boost/asio.hpp>

namespace cacos::test {

Generator::Generator(const GeneratorOptions& opts)
    : opts_(opts) {
}

Generator::Generator(GeneratorOptions&& opts)
    : opts_(std::move(opts)) {
}

void Generator::run() {
    executable::Executable exe(opts_.workspace / opts_.generator);

    InlineVariables vars;

    std::vector<bp::child> children(std::thread::hardware_concurrency());

    executable::Flags flags(opts_.args);
    executable::ExecPool pool;

    std::vector<std::unique_ptr<std::string>> input;
    std::vector<std::unique_ptr<std::string>> output;
    traverse(opts_.vars.begin(), vars, [&](const InlineVariables& vars) {
        // std::string name = vars.parse(opts_.testName);
        auto res = flags.build(vars);
        input.emplace_back(std::make_unique<std::string>(vars.parse(opts_.input)));
        output.emplace_back(std::make_unique<std::string>(4096, '\0'));

        executable::ExecTask task {
            exe,
            bp::buffer(std::as_const(*input.back())),
            bp::buffer(*output.back()),
            res
        };

        pool.push(std::move(task));
    });

    pool.run();

    for (size_t i = 0; i < input.size(); ++i) {
        std::cout << *input[i] << ": " << *output[i] << std::endl;
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
