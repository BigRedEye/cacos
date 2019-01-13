#include "cacos/util/mt/fixed_queue.h"

namespace cacos::mt {

FixedQueue::FixedQueue(size_t threads)
    : threads_(threads)
{}

void FixedQueue::add(const Task& task) {
    tasks_.push_back(task);
}

void FixedQueue::add(Task&& task) {
    tasks_.push_back(std::move(task));
}

void FixedQueue::run() {
    next_.store(0);
    for (size_t i = 0; i < std::min(tasks_.size(), threads_); ++i) {
        workers_.emplace_back([this] {
            worker();
        });
    }
}

void FixedQueue::wait() {
    for (auto&& thread : workers_) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    workers_.clear();
    tasks_.clear();
}

void FixedQueue::worker() {
    while (true) {
        size_t cur = next_.fetch_add(1);
        if (cur >= tasks_.size()) {
            break;
        }

        tasks_[cur]();
    }
}

} // namespace cacos::mt
