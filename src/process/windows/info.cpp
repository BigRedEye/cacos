#ifdef CACOS_OS_WINDOWS

#include "cacos/process/info.h"

#include "cacos/util/ints.h"
#include "cacos/util/split.h"
#include "cacos/util/string.h"

#include <psapi.h>
#include <windows.h>

namespace cacos::process {

InfoError::InfoError(const std::string& msg)
    : std::runtime_error(msg) {
}

class InfoFetcher::Impl {
public:
    Impl(const bp::child& c) {
        handle_ = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, TRUE, c.id());
        if (!handle_) {
            throw InfoError("Cannot open process");
        }
    }

    Info get() {
        try {
            update();
        } catch (const InfoError&) {
        }
        return cache_;
    }

    void status(status::Status newStatus) {
        cache_.result = newStatus;
    }

private:
    void update() {
        PROCESS_MEMORY_COUNTERS memory;
        if (!GetProcessMemoryInfo(handle_, &memory, sizeof(memory))) {
            throw InfoError(util::join("Cannot get process memory usage: winapi last_error = ", GetLastError()));
        }

        bytes rss = static_cast<bytes>(memory.PeakWorkingSetSize);

        FILETIME creationTime;
        FILETIME exitTime;
        FILETIME kernelTime;
        FILETIME userTime;
        if (!GetProcessTimes(handle_, &creationTime, &exitTime, &kernelTime, &userTime)) {
            throw InfoError(util::join("Cannot get process times: winapi last_error = ", GetLastError()));
        }

        auto fuckingWinapiStructureToSeconds = [] (FILETIME time) {
            ui64 idioticTicks = (ui64{time.dwHighDateTime} << 32) + time.dwLowDateTime;
            ui64 nanoseconds = idioticTicks * 100;
            return std::chrono::duration_cast<seconds>(std::chrono::nanoseconds(nanoseconds));
        };

        SYSTEMTIME currentSystemTime;
        GetSystemTime(&currentSystemTime);
        FILETIME currentTime;
        SystemTimeToFileTime(&currentSystemTime, &currentTime);

        cache_.maxRss = std::max<ui64>(cache_.maxRss, rss);
        cache_.cpuTime = fuckingWinapiStructureToSeconds(kernelTime) + fuckingWinapiStructureToSeconds(userTime);
        cache_.realTime = fuckingWinapiStructureToSeconds(currentTime) - fuckingWinapiStructureToSeconds(creationTime);
    }

private:
    Info cache_;
    HANDLE handle_;
};

InfoFetcher::InfoFetcher(const bp::child& c)
    : impl_(std::make_unique<Impl>(c)) {
}

InfoFetcher::~InfoFetcher() {
}

Info InfoFetcher::get() const {
    return impl_->get();
}

void InfoFetcher::status(status::Status newStatus) {
    return impl_->status(newStatus);
}

} // namespace cacos::process

#endif // CACOS_OS_WINDOWS
