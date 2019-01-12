#include "cacos/test/generate/generator.h"

#include "cacos/executable/executable.h"

#include "cacos/util/mt/fixed_queue.h"
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

    mt::FixedQueue queue;

    InlineVariables vars;

    std::mutex mtx;

    traverse(opts_.vars.begin(), vars, [&](const InlineVariables& vars) {
        executable::Flags flags(opts_.args);
        std::string in = vars.parse(opts_.input);
        auto res = flags.build(vars);
        queue.add([&, args = std::move(res), input = std::move(in)] {

            // std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 2000));
            static thread_local boost::asio::io_service ios;

            // std::cout << "Running, input = " << input << ", id = 0x" << std::hex << std::this_thread::get_id() << std::endl;
            std::string out(128, '\0');
            mtx.lock();
            auto child = exe.run(args, bp::std_in < bp::buffer(input), bp::std_out > bp::buffer(out), bp::std_err > bp::null, ios);
            mtx.unlock();
            // std::cout << "Running ios, id = 0x" << std::hex << std::this_thread::get_id() << std::endl;
            ios.restart();
            ios.run();
            // std::cout << "Joining child, id = 0x" << std::hex << std::this_thread::get_id() << std::endl;
            if (child.joinable()) {
                child.join();
            }

            // std::unique_lock<std::mutex> lock(mtx);
            // std::cout << input << ": " << out << std::endl;
        });
    });

    queue.run();
    queue.wait();
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
