
#ifdef CACOS_OS_UNIX

#include "cacos/process/process.h"
#include "cacos/util/string.h"
#include "cacos/util/split.h"

#include <cstdio>

#include <sys/types.h>
#include <unistd.h>

namespace cacos::process {

namespace proc::stat {

/* man 5 proc */
enum {
    UTIME = 13,
    STIME = 14,
    CUTIME = 15,
    CSTIME = 16,
    STARTTIME = 21,
    RSS = 23,
};

}

InfoError::InfoError(const std::string& msg)
    : std::runtime_error(msg)
{}

struct ProcessStats {
    ui64 utime;
    ui64 stime;
    ui64 cutime;
    ui64 cstime;
    ui64 starttime;
    ui64 rss;
};

class Process::Impl {
public:
    Impl(bp::child&& c)
        : child_(std::move(c))
    {}

    Info info() const {
        ProcessStats stats = parseStats();

        ui64 total = stats.stime + stats.utime + stats.cstime + stats.cutime;
        static const long ticks = sysconf(_SC_CLK_TCK);
        double uptime = parseUptime();

        bytes rss = stats.rss * getpagesize();
        cachedInfo_.maxRss = std::max<ui64>(cachedInfo_.maxRss, rss);
        cachedInfo_.cpuTime = seconds(total / static_cast<double>(ticks));
        cachedInfo_.realTime = seconds(uptime - stats.starttime / static_cast<double>(ticks));

        return cachedInfo_;
    }

    Info cachedInfo() const {
        try {
            return info();
        } catch (const InfoError&) {
            return cachedInfo_;
        }
    }

    bool valid() {
        return child_.valid();
    }

    bool running() {
        return child_.running();
    }

    void wait() {
        std::error_code errc;
        return child_.wait(errc);
    }

    void terminate(status::Status reason) {
        cachedInfo_.result = reason;
        return child_.terminate();
    }

private:
    ProcessStats parseStats() const {
        std::string pid = util::to_string(child_.id());
        fs::path statPath = fs::path("/proc") / pid / "stat";
        if (!fs::exists(statPath)) {
            throw InfoError("Cannot open " + statPath.string() + " file");
        }

        std::ifstream ifs(statPath);
        std::string raw = util::readFile(ifs);
        std::vector<std::string_view> tokens = util::split(raw, " ");

        ProcessStats stats;
        stats.rss = util::from_string<ui64>(tokens[proc::stat::RSS]);
        stats.utime = util::from_string<ui64>(tokens[proc::stat::UTIME]);
        stats.stime = util::from_string<ui64>(tokens[proc::stat::STIME]);
        stats.cutime = util::from_string<ui64>(tokens[proc::stat::CUTIME]);
        stats.cstime = util::from_string<ui64>(tokens[proc::stat::CSTIME]);
        stats.starttime = util::from_string<ui64>(tokens[proc::stat::STARTTIME]);

        return stats;
    }

    double parseUptime() const {
        fs::path statPath = fs::path("/proc") / "uptime";
        if (!fs::exists(statPath)) {
            throw InfoError("Cannot open " + statPath.string() + " file");
        }

        std::ifstream ifs(statPath);

        double seconds;
        ifs >> seconds;
        return seconds;
    }

private:
    mutable Info cachedInfo_;

    bp::child child_;
};

Process::Process(bp::child&& c)
    : impl_(std::make_unique<Impl>(std::move(c)))
{}

Process::~Process() {
}

Info Process::info() const {
    return impl_->info();
}

Info Process::cachedInfo() const {
    return impl_->cachedInfo();
}

bool Process::valid() {
    return impl_->valid();
}

bool Process::running() {
    return impl_->running();
}

void Process::terminate(status::Status reason) {
    return impl_->terminate(reason);
}

void Process::wait() {
    return impl_->wait();
}

}

#endif
