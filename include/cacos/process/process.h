#pragma once

#include "cacos/process/info.h"
#include "cacos/process/result.h"

#include "cacos/util/util.h"

#include <chrono>
#include <stdexcept>

namespace cacos::process {

class Process {
public:
    Process(bp::child&& child);

    Process(Process&& rhs) = default;
    Process(const Process& rhs) = delete;

    Process& operator=(Process&& rhs) = default;
    Process& operator=(const Process& rhs) = delete;

    Info info() const;

    bool valid();
    bool running();
    void wait();
    void kill(status::Status reason, std::chrono::microseconds timeout = defaultTimeout);

private:
    static constexpr std::chrono::microseconds defaultTimeout{0};

    bp::child child_;
    InfoFetcher fetcher_;
};

}
