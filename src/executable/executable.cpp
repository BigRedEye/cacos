#include "cacos/executable/executable.h"

#include <boost/asio.hpp>

namespace cacos::executable {

Executable::Executable(const fs::path& exe)
    : Executable(exe, {})
{}

Executable::Executable(const fs::path& exe, const std::vector<std::string>& flags)
    : executable_(exe)
    , flags_(flags)
{}

ExecTask::~ExecTask() {
}

ExecPool::ExecPool(size_t workers)
    : workers_(workers)
{}

void ExecPool::push(ExecTaskPtr&& task) {
    tasks_.push_back(std::move(task));
}

void ExecPool::run() {
    boost::asio::io_context ctx;

    bp::group children;

    auto task = tasks_.begin();

    size_t runningTasks = 0;

    auto onExit = [&] (int, const std::error_code&) {
        --runningTasks;
    };

    std::vector<bp::child> child;
    auto pushTask = [&] {
        if (task == tasks_.end()) {
            return false;
        }

        bp::child c = (*task)->run(onExit, {children, ctx});
        ++task;
        ++runningTasks;

        child.push_back(std::move(c));
        return true;
    };

    while (task != tasks_.end()) {
        pushTask();

        while (runningTasks >= workers_) {
            ctx.restart();
            ctx.run_for(std::chrono::milliseconds(1));
            std::error_code errc;
            children.wait_for(std::chrono::milliseconds(1), errc);
        }
    }

    ctx.restart();
    ctx.run();

    for (auto&& c : child) {
        if (c.running()) {
            c.wait();
        }
    }

    if (children.joinable()) {
        std::error_code errc;
        children.wait(errc);
    }

    tasks_.clear();
}

}
