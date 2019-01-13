#pragma once

#include "cacos/process/result.h"

#include "cacos/util/util.h"

#include <chrono>
#include <stdexcept>

namespace cacos::process {

class InfoError : public std::runtime_error {
public:
    InfoError(const std::string& s);
};

struct Info {
    bytes maxRss = 0;
    seconds cpuTime = seconds(0.);
    seconds realTime = seconds(0.);
    status::Status result = status::OK;
};

class Process {
public:
    Process(bp::child&& child);
    ~Process();

    Process(Process&& rhs) = default;
    Process(const Process& rhs) = delete;

    Process& operator=(Process&& rhs) = default;
    Process& operator=(const Process& rhs) = delete;

    Info info() const;
    Info cachedInfo() const;

    bool valid();
    bool running();
    void wait();
    void terminate(status::Status reason);

private:
    class Impl;

    std::unique_ptr<Impl> impl_;
};

}
