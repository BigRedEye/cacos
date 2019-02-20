#pragma once

#include <iostream>

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace cacos::log {

namespace detail {

class Logger {
public:
    enum class MessagePriority { debug, log, info, warning, error, fatal };

    explicit Logger(MessagePriority prior = MessagePriority::info);

    Logger(const Logger& other) = delete;
    Logger(Logger&& other) = delete;

    Logger& operator=(const Logger& other) = delete;
    Logger& operator=(Logger&& other) = delete;

    ~Logger();

    Logger& delimer(char delim);
    Logger& flush(bool flush);

    template<typename T>
    Logger& operator<<(const T& msg) {
        if (verbosity <= prior_) {
            os_ << msg;
            if (delim_) {
                os_ << delim_;
            }
        }
        return *this;
    }

    template<typename... Args>
    Logger& print(Args&&... args) {
        if (verbosity <= prior_) {
            fmt::print(os_, std::forward<Args>(args)...);
        }
        return *this;
    }

    Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        if (verbosity <= prior_) {
            os_ << manip;
        }
        return *this;
    }

    static void increaseVerbosity(int delta = 1);

private:
    static MessagePriority verbosity;

    std::ostream& os_;
    MessagePriority prior_;
    char delim_;
    bool flush_;
};

} // namespace detail

detail::Logger debug();
detail::Logger log();
detail::Logger info();
detail::Logger warning();
detail::Logger error();
detail::Logger fatal();

} // namespace cacos::log
