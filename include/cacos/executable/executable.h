#pragma once

#include "cacos/executable/flags.h"

#include "cacos/process/limits.h"
#include "cacos/process/process.h"

#include "cacos/util/inline_variables.h"
#include "cacos/util/util.h"

#include <variant>

namespace cacos::executable {

class Executable {
public:
    Executable() = default;
    Executable(const fs::path& exe);
    Executable(const fs::path& exe, const std::vector<std::string>& flags);

    bool operator==(const Executable& other) const;
    bool operator!=(const Executable& other) const;

    template<typename... Args>
    bp::child run(const InlineVariables& vars, Args&&... args) const {
        bp::child result(
            bp::exe = executable_.string(),
            bp::args += flags_.build(vars),
            std::forward<Args>(args)...);
        return result;
    }

    template<typename... Args>
    bp::child run(const std::vector<std::string>& flags, Args&&... args) const {
        bp::child result(
            bp::exe = executable_.string(),
            bp::args += flags_.build({}),
            bp::args += flags,
            std::forward<Args>(args)...);
        return result;
    }

    const fs::path& path() const;

private:
    fs::path executable_;
    Flags flags_;
};

struct ExecTaskContext {
    using Callback = std::function<void(process::Result, std::optional<process::Info>&&)>;

    std::vector<std::string> args{};
    bp::environment env = boost::this_process::environment();
    Callback callback{};
};

class ExecTask {
public:
    ExecTask(const Executable& exe, ExecTaskContext&& ctx = {})
        : exe_(exe)
        , ctx_(ctx) {
    }

    virtual ~ExecTask();

    virtual bp::child run(
        const std::function<void(int, const std::error_code&)>& exitCallback,
        boost::asio::io_context& ctx) = 0;

    void onExit(process::Result result, std::optional<process::Info>&& info);

protected:
    const Executable& exe_;
    ExecTaskContext ctx_;
};

using ExecTaskPtr = std::unique_ptr<ExecTask>;

template<
    typename StdIn = bp::detail::null_t,
    typename StdOut = bp::detail::null_t,
    typename StdErr = bp::detail::null_t>
class ExecTaskImpl final : public ExecTask {
public:
    ExecTaskImpl(const Executable& exe, ExecTaskContext&& ctx, StdIn in, StdOut out, StdErr err)
        : ExecTask(exe, std::move(ctx))
        , stdin_(in)
        , stdout_(out)
        , stderr_(err) {
    }

    bp::child run(
        const std::function<void(int, const std::error_code&)>& exitCallback,
        boost::asio::io_context& ioctx) override {
        return exe_.run(
            ctx_.args,
            ctx_.env,
            bp::std_in = stdin_,
            bp::std_out = stdout_,
            bp::std_err = stderr_,
            bp::on_exit = exitCallback,
            ioctx);
    }

private:
    StdIn stdin_;
    StdOut stdout_;
    StdErr stderr_;
};

/*
 * gcc bug
 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=81486
 */
template<typename... Args>
ExecTaskPtr makeTask(const Executable& exe, ExecTaskContext&& ctx, Args&&... rest) {
    return ExecTaskPtr(new ExecTaskImpl<std::decay_t<Args>...>(exe, std::move(ctx), std::forward<Args>(rest)...));
}

template<typename... Args>
ExecTaskPtr makeTask(const Executable& exe, const ExecTaskContext& ctx, Args&&... rest) {
    ExecTaskContext copy = ctx;
    return maskTask(exe, std::move(copy), std::forward<Args>(rest)...);
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

} // namespace cacos::executable
