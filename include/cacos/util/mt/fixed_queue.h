#pragma once

#include <atomic>
#include <exception>
#include <functional>
#include <thread>
#include <vector>

namespace cacos::util::mt {

using Task = std::function<void(void)>;

class FixedQueue {
public:
    FixedQueue(size_t threads = std::thread::hardware_concurrency());

    void add(const Task& task);
    void add(Task&& task);

    void start();
    void wait();

    void run() {
        start();
        wait();
    }

private:
    void worker(size_t idx);

private:
    size_t threads_;
    std::vector<std::thread> workers_;
    std::vector<std::exception_ptr> exceptions_;

    std::vector<Task> tasks_;
    std::atomic_size_t next_;
};

} // namespace cacos::util::mt
