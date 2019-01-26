#include "cacos/process/process.h"
#include "cacos/util/split.h"
#include "cacos/util/string.h"

#include <cstdio>

#ifdef CACOS_OS_UNIX
#include <signal.h>
#endif

namespace cacos::process {

Process::Process(bp::child&& c)
    : child_(std::move(c))
    , fetcher_(child_) {
}

Info Process::info() const {
    return fetcher_.get();
}

bool Process::valid() {
    return child_.valid();
}

bool Process::running() {
    return child_.running();
}

void Process::kill(status::Status reason, std::chrono::microseconds timeout) {
    fetcher_.get();
    fetcher_.status(reason);

#ifdef CACOS_OS_UNIX
    ::kill(-child_.id(), SIGTERM);
#endif // CACOS_OS_UNIX

    /* give process some time to finish */
    std::error_code errc;
    if (!child_.wait_for(timeout, errc)) {
        child_.terminate();
    }
}

void Process::wait() {
    std::error_code errc;
    return child_.wait(errc);
}

} // namespace cacos::process
