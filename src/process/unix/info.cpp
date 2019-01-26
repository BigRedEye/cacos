#ifdef CACOS_OS_UNIX

#include "cacos/process/info.h"

#include "cacos/util/ints.h"
#include "cacos/util/split.h"
#include "cacos/util/string.h"

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

} // namespace proc::stat

InfoError::InfoError(const std::string& msg)
    : std::runtime_error(msg) {
}

struct ProcessStats {
    ui64 utime;
    ui64 stime;
    ui64 cutime;
    ui64 cstime;
    ui64 starttime;
    ui64 rss;
};

class InfoFetcher::Impl {
public:
    Impl(const bp::child& c)
        : pid_(c.id()) {
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
        ProcessStats stats = parseStats();

        ui64 total = stats.stime + stats.utime + stats.cstime + stats.cutime;
        static const long ticks = sysconf(_SC_CLK_TCK);
        double uptime = parseUptime();

        bytes rss = stats.rss * static_cast<ui64>(getpagesize());
        cache_.maxRss = std::max<ui64>(cache_.maxRss, rss);
        cache_.cpuTime = seconds(total / static_cast<double>(ticks));
        cache_.realTime = seconds(uptime - stats.starttime / static_cast<double>(ticks));
    }

    ProcessStats parseStats() {
        std::string pid = util::to_string(pid_);
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
    Info cache_;
    pid_t pid_;
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

#endif // CACOS_OS_UNIX
