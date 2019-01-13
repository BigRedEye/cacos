#pragma once

#include "cacos/executable/flags.h"

#include "cacos/util/util.h"
#include "cacos/util/inline_variables.h"

#include <variant>

namespace cacos::executable {

class Executable {
public:
    Executable(const fs::path& exe);
    Executable(const fs::path& exe, const std::vector<std::string>& flags);

    template<typename ...Args>
    bp::child run(
        const InlineVariables& vars,
        Args&&... args) {
        bp::child result(bp::exe = executable_.string(), bp::args += flags_.build(vars), std::forward<Args>(args)...);
        return result;
    }

    template<typename ...Args>
    bp::child run(
        const std::vector<std::string>& flags,
        Args&&... args) {
        bp::child result(bp::exe = executable_.string(), bp::args += flags, std::forward<Args>(args)...);
        return result;
    }

private:
    fs::path executable_;
    Flags flags_;
};

using InputBuffer = std::variant<boost::asio::const_buffer, fs::path>;
using OutputBuffer = std::variant<boost::asio::mutable_buffer, fs::path>;

struct ExecTask {
    Executable& exe;
    boost::asio::const_buffer input;
    boost::asio::mutable_buffer output;
    std::vector<std::string> args;
};

class ExecPool {
public:
    ExecPool(size_t workers = std::thread::hardware_concurrency());

    void push(ExecTask&& task);
    void run();

private:
    size_t workers_;
    std::vector<ExecTask> tasks_;
};

}
