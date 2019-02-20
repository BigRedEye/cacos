#include "cacos/util/mt/fixed_queue.h"

#include <algorithm>

namespace cacos::util::mt {

class WorkerException : public std::runtime_error {
public:
    WorkerException()
        : std::runtime_error("Exception from worker thread") {
    }
};

FixedQueue::FixedQueue(size_t threads)
    : threads_(threads) {
}

void FixedQueue::add(const Task& task) {
    tasks_.push_back(task);
}

void FixedQueue::add(Task&& task) {
    tasks_.push_back(std::move(task));
}

void FixedQueue::start() {
    next_.store(0);
    size_t workers = std::min(tasks_.size(), threads_);
    exceptions_.assign(workers, nullptr);
    for (size_t i = 0; i < workers; ++i) {
        workers_.emplace_back([this, i] { worker(i); });
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

    for (auto& e : exceptions_) {
        if (e) {
            auto ptr = std::move(e);
            e = nullptr;

            try {
                std::rethrow_exception(ptr);
            } catch (...) {
                std::throw_with_nested(WorkerException{});
            }
        }
    }

    exceptions_.clear();
}

void FixedQueue::worker(size_t idx) {
    try {
        while (true) {
            size_t cur = next_.fetch_add(1);
            if (cur >= tasks_.size()) {
                break;
            }

            tasks_[cur]();
        }
    } catch (...) {
        exceptions_[idx] = std::current_exception();
    }
}

} // namespace cacos::util::mt
