#include "cacos/executable/executable.h"
#include "cacos/process/process.h"

#include "cacos/lang/lang.h"

#include "cacos/util/logger.h"

#include <boost/asio.hpp>

#include <unordered_map>

namespace cacos::executable {

Executable::Executable(const fs::path& exe)
    : Executable(exe, {}) {
}

Executable::Executable(const fs::path& exe, const std::vector<std::string>& flags)
    : executable_(exe)
    , flags_(flags) {
}

bool Executable::operator==(const Executable& other) const {
    return executable_ == other.executable_ && flags_ == other.flags_;
}

bool Executable::operator!=(const Executable& other) const {
    return executable_ == other.executable_ && flags_ == other.flags_;
}

const fs::path& Executable::path() const {
    return executable_;
}

ExecTask::~ExecTask() {
}

void ExecTask::onExit(process::Result res, std::optional<process::Info>&& info) {
    if (ctx_.callback) {
        ctx_.callback(res, std::move(info));
    }
}

ExecPool::ExecPool(process::Limits limits, size_t workers)
    : workers_(workers)
    , limits_(limits) {
}

void ExecPool::push(ExecTaskPtr&& task) {
    tasks_.push_back(std::move(task));
}

/*
 * Ехал Лямбда через лямбду
 * Видит Лямбда - в лямбде лямбда
 * Сунул в лямбду лямбду Лямбда
 * Лямбда лямбда Лямбду лямбда
 */
void ExecPool::run() {
    boost::asio::io_context ctx;

    auto task = tasks_.begin();

    size_t runningTasks = 0;

    std::unordered_map<ui64, process::Process> running;
    ui64 id = 0;
    auto pushTask = [&] {
        if (task == tasks_.end()) {
            return false;
        }

        auto onExit = [&, id, task = (*task).get()](int exitCode, const std::error_code&) {
            auto it = running.find(id);
            std::optional<process::Info> info;
            if (it != running.end()) {
                info = it->second.info();
            }
            process::Result res{info ? info->result : process::status::UNDEFINED, exitCode};
            task->onExit(res, std::move(info));

            if (it != running.end()) {
                running.erase(it);
            }

            --runningTasks;
        };

        bp::child c = (*task)->run(onExit, {ctx});

        running.emplace(id, std::move(c));

        ++task;
        ++runningTasks;
        ++id;

        return true;
    };

    auto pollInfo = [&] {
        for (auto&& [i, c] : running) {
            process::Info info = c.info();
            Logger::debug().print(
                "cpu time = {:.3f} s, real time = {:.3f} s, max rss = {:.3f} mb",
                info.cpuTime.count(),
                info.realTime.count(),
                info.maxRss / (1024. * 1024.));
            if (limits_.cpu != process::Limits::unlimited<seconds> && info.cpuTime > limits_.cpu) {
                c.kill(process::status::TL);
            }
            if (limits_.real != process::Limits::unlimited<seconds> &&
                info.realTime > limits_.real) {
                c.kill(process::status::IL);
            }
            if (limits_.ml != process::Limits::unlimited<bytes> && info.maxRss > limits_.ml) {
                c.kill(process::status::ML);
            }
        }
    };

    auto poll = [&](std::chrono::microseconds timeout) {
        ctx.restart();
        ctx.run_for(timeout);
        pollInfo();
    };

    while (task != tasks_.end()) {
        pushTask();

        while (runningTasks >= workers_) {
            poll(defaultTimeout);
        }
    }

    while (runningTasks > 0) {
        poll(defaultTimeout);
    }

    ctx.restart();
    ctx.run();

    if (!running.empty()) {
        Logger::error() << "Running tasks " << running.size() << " != 0: ";
    }

    tasks_.clear();
}

} // namespace cacos::executable
