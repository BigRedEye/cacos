#pragma once

#include "cacos/process/result.h"
#include "cacos/util/util.h"

#include <boost/process/child.hpp>

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

class InfoFetcher {
public:
    InfoFetcher(const bp::child& child);
    ~InfoFetcher();

    Info get() const;
    void status(status::Status newStatus);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace cacos::process
