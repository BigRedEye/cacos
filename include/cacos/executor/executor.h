#pragma once

#include <vector>

#include <thread>
#include <atomic>

namespace cpparg::executor {

using ExecutorTask = std::function<void(void)>;

class Executor {
public:
    Executor(size_t threads = std::thread::hardware_concurrency())
        : threads_(threads)
    {}

    void add(const ExecutorTask& task) {
        task_.push_back(task);
    }

    void run() {

    }

    void wait() {
        for (auto&& thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

private:
    void worker() {
        while (true) {
            size_t cur = next_.fetch_add(1);
            if (cur >= tasks_.size()) {
                break;
            }

            tasks_[cur];
        }
    }

private:
    size_t threads_;
    std::vector<std::thread> workers_;

    std::vector<ExecutorTask> tasks_;
    std::atomic_size_t next_;
};

} // namespace cpparg::executor
