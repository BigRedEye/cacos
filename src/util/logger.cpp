#include "cacos/util/logger.h"

#include <termcolor/termcolor.hpp>

#include <chrono>
#include <cstdarg>
#include <iomanip>
#include <iostream>
#include <string>

namespace cacos {

namespace  {

std::ostream& boldRed(std::ostream& os) {
    return termcolor::bold(termcolor::red(os));
}

auto colorForPriority(Logger::MessagePriority priority) {
    switch (priority) {
    case Logger::LOG:
        return termcolor::white;
    case Logger::INFO:
        return termcolor::reset;
    case Logger::WARNING:
        return termcolor::yellow;
    case Logger::ERROR:
        return termcolor::red;
    case Logger::FATAL:
        return boldRed;
    default:
        break;
    }
}

}

Logger::Logger(Logger::MessagePriority priority)
    : os_(std::cerr)
    , prior_(priority)
    , delim_(0)
    , flush_(true) {
    *this << termcolor::reset << colorForPriority(priority) << "[ ";

    switch (priority) {
    case Logger::LOG:
        *this << "LOG";
        break;
    case Logger::INFO:
        *this << "INFO";
        break;
    case Logger::WARNING:
        *this << "WARNING";
        break;
    case Logger::ERROR:
        *this << "ERROR";
        break;
    case Logger::FATAL:
        *this << "FATAL";
        break;
    default:
        break;
    }
    *this << " ] ";
}

Logger::~Logger() {
    os_ << termcolor::reset;
    if (flush_ || prior_ >= ERROR) {
        os_ << std::endl;
    } else {
        *this << '\n';
    }
}

Logger& Logger::delimer(char delim) {
    delim_ = delim;
    return *this;
}

Logger& Logger::flush(bool flush) {
    flush_ = flush;
    return *this;
}

} // namespace cacos
