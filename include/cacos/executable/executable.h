#pragma once

#include "cacos/executable/flags.h"

#include "cacos/process/process.h"
#include "cacos/process/limits.h"

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

struct ExecTaskContext {
    boost::asio::io_context& ctx;
};

class ExecTask {
public:
    using Callback = std::function<void(process::Result, std::optional<process::Info>&&)>;

    ExecTask(Executable& exe, Callback&& callback = {}, std::vector<std::string> args = {})
        : exe_(exe)
        , callback_(std::move(callback))
        , args_(std::move(args))
    {}

    virtual ~ExecTask();

    virtual bp::child run(
        const std::function<void(int, const std::error_code&)>& exitCallback,
        const ExecTaskContext& ctx) = 0;

    void onExit(process::Result result, std::optional<process::Info>&& info);

protected:
    Executable& exe_;
    Callback callback_;
    std::vector<std::string> args_;
};

using ExecTaskPtr = std::unique_ptr<ExecTask>;

template<typename StdIn = bp::detail::null_t, typename StdOut = bp::detail::null_t, typename StdErr = bp::detail::null_t>
class ExecTaskImpl final : public ExecTask {
public:
    ExecTaskImpl(Executable& exe, Callback&& callback, std::vector<std::string> args, StdIn in, StdOut out, StdErr err)
        : ExecTask(exe, std::move(callback), std::move(args))
        , stdin_(in)
        , stdout_(out)
        , stderr_(err)
    {}

    ExecTaskImpl(Executable& exe, std::vector<std::string> args, StdIn in, StdOut out, StdErr err)
        : ExecTask(exe, {}, std::move(args))
        , stdin_(in)
        , stdout_(out)
        , stderr_(err)
    {}

    bp::child run(
        const std::function<void(int, const std::error_code&)>& exitCallback,
        const ExecTaskContext& ctx) override {
        return exe_.run(
            args_,
            bp::std_in = stdin_,
            bp::std_out = stdout_,
            bp::std_err = stderr_,
            bp::on_exit = exitCallback,
            ctx.ctx
        );
    }

private:
    StdIn stdin_;
    StdOut stdout_;
    StdErr stderr_;
};

template<typename ...Args>
ExecTaskPtr makeTask(Executable& exe, const std::vector<std::string>& args, Args&&... rest) {
    return ExecTaskPtr(new ExecTaskImpl(exe, args, std::forward<Args>(rest)...));
}

template<typename ...Args>
ExecTaskPtr makeTask(Executable& exe, ExecTask::Callback callback, const std::vector<std::string>& args, Args&&... rest) {
    return ExecTaskPtr(new ExecTaskImpl(exe, std::move(callback), args, std::forward<Args>(rest)...));
}

class ExecPool {
public:
    ExecPool(process::Limits limits = {}, size_t workers = defaultWorkers());

    void push(ExecTaskPtr&& task);
    void run();

    static size_t defaultWorkers() {
        return std::max<size_t>(std::thread::hardware_concurrency(), 2) - 1;
    }

private:
    static constexpr std::chrono::microseconds defaultTimeout = std::chrono::milliseconds(1);

    size_t workers_;
    process::Limits limits_;

    std::vector<ExecTaskPtr> tasks_;
};

}
