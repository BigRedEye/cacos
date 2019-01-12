#pragma once

#include <iostream>

namespace cacos {

class Logger {
public:
    enum MessagePriority { LOG, INFO, WARNING, ERROR, FATAL };

    Logger(MessagePriority prior = INFO);

    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& delimer(char delim);
    Logger& flush(bool flush);

    template<typename T>
    inline Logger& operator<<(const T& msg) {
        os_ << msg;
        if (delim_) {
            os_ << delim_;
        }
        return *this;
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

private:
    std::ostream& os_;

    MessagePriority prior_;
    char delim_;
    bool flush_;
};

} // namespace cacos
