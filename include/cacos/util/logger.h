#pragma once

#include <iostream>

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace cacos {

class Logger {
public:
    enum MessagePriority { DEBUG, LOG, INFO, WARNING, ERROR, FATAL };

    Logger(MessagePriority prior = INFO);

    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& delimer(char delim);
    Logger& flush(bool flush);

    template<typename T>
    Logger& operator<<(const T& msg) {
        if (verbosity_ <= prior_) {
            os_ << msg;
            if (delim_) {
                os_ << delim_;
            }
        }
        return *this;
    }

    template<typename... Args>
    Logger& print(Args&&... args) {
        if (verbosity_ <= prior_) {
            fmt::print(os_, std::forward<Args>(args)...);
        }
        return *this;
    }

    Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        if (verbosity_ <= prior_) {
            os_ << manip;
        }
        return *this;
    }

    static Logger debug() {
        return Logger(DEBUG);
    }

    static Logger log() {
        return Logger(LOG);
    }

    static Logger info() {
        return Logger(INFO);
    }

    static Logger warning() {
        return Logger(WARNING);
    }

    static Logger error() {
        return Logger(ERROR);
    }

    static Logger fatal() {
        return Logger(FATAL);
    }

    static void increaseVerbosity(int delta = 1);

private:
    static MessagePriority verbosity_;

    std::ostream& os_;
    MessagePriority prior_;
    char delim_;
    bool flush_;
};

} // namespace cacos
