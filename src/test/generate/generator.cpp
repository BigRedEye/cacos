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

template<typename T>
class FutureRef {
public:
    FutureRef(std::future<T>& future)
        : future_(future)
    {}

    operator std::future<T>&() {
        return future_;
    }

private:
    std::future<T>& future_;
};

void Generator::run() {
    executable::Executable exe(opts_.workspace / opts_.generator);

    InlineVariables vars;

    std::vector<bp::child> children(std::thread::hardware_concurrency());

    executable::Flags flags(opts_.args);
    executable::ExecPool pool;

    std::vector<std::unique_ptr<std::string>> input;
    std::vector<std::unique_ptr<std::future<std::string>>> output;
    traverse(opts_.vars.begin(), vars, [&](const InlineVariables& vars) {
        auto res = flags.build(vars);
        input.emplace_back(std::make_unique<std::string>(vars.parse(opts_.input)));
        output.emplace_back(std::make_unique<std::future<std::string>>());

        auto task = executable::makeTask(
            exe,
            res,
            bp::buffer(std::as_const(*input.back())),
            FutureRef(*output.back()),
            bp::null
        );

        pool.push(std::move(task));
    });

    pool.run();

    for (size_t i = 0; i < input.size(); ++i) {
        std::cout << *input[i] << ": " << (*output[i]).get() << std::endl;
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
