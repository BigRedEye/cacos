#pragma once

#include <vector>

#include <atomic>
#include <thread>

#include <functional>

namespace cacos::mt {

using Task = std::function<void(void)>;

class FixedQueue {
public:
    FixedQueue(size_t threads = std::thread::hardware_concurrency());

    void add(const Task& task);
    void add(Task&& task);
    void run();
    void wait();

private:
    void worker();

private:
    size_t threads_;
    std::vector<std::thread> workers_;

    std::vector<Task> tasks_;
    std::atomic_size_t next_;
};

} // namespace cacos::mt
