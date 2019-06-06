#include "cacos/util/logger.h"

#include <termcolor/termcolor.hpp>

#include <chrono>
#include <cstdarg>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

namespace cacos::log {

namespace detail {

Logger::MessagePriority Logger::verbosity = Logger::MessagePriority::info;

namespace {

std::ostream& boldRed(std::ostream& os) {
    return termcolor::bold(termcolor::red(os));
}

auto colorForPriority(Logger::MessagePriority priority) {
    switch (priority) {
    case Logger::MessagePriority::debug:
        return termcolor::dark;
    case Logger::MessagePriority::log:
        return termcolor::white;
    case Logger::MessagePriority::info:
        return termcolor::reset;
    case Logger::MessagePriority::warning:
        return termcolor::yellow;
    case Logger::MessagePriority::error:
        return termcolor::red;
    case Logger::MessagePriority::fatal:
        return boldRed;
    default:
        break;
    }
    return +[](std::ostream & os) -> auto& {
        return os;
    };
}

} // namespace

Logger::Logger(Logger::MessagePriority priority)
    : os_(std::cerr)
    , prior_(priority)
    , delim_(0)
    , flush_(true) {
    *this << termcolor::reset << colorForPriority(priority) << "[ ";

    switch (priority) {
    case Logger::MessagePriority::debug:
        *this << "DEBUG";
        break;
    case Logger::MessagePriority::log:
        *this << "LOG";
        break;
    case Logger::MessagePriority::info:
        *this << "INFO";
        break;
    case Logger::MessagePriority::warning:
        *this << "WARNING";
        break;
    case Logger::MessagePriority::error:
        *this << "ERROR";
        break;
    case Logger::MessagePriority::fatal:
        *this << "FATAL";
        break;
    default:
        break;
    }
    *this << " ] ";
    if (static_cast<int>(priority) <= static_cast<int>(Logger::MessagePriority::debug)) {
        *this << "[ 0x" << std::hex << std::this_thread::get_id() << std::dec << " ] ";
    }
    *this << termcolor::reset;
}

Logger::~Logger() {
    *this << termcolor::reset;
    if (flush_) {
        *this << std::endl;
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

void Logger::increaseVerbosity(int delta) {
    verbosity = static_cast<MessagePriority>(std::max<int>(
        static_cast<int>(verbosity) - delta, static_cast<int>(MessagePriority::debug)));
}

} // namespace detail

detail::Logger debug() {
    return detail::Logger(detail::Logger::MessagePriority::debug);
}

detail::Logger log() {
    return detail::Logger(detail::Logger::MessagePriority::log);
}

detail::Logger info() {
    return detail::Logger(detail::Logger::MessagePriority::info);
}

detail::Logger warning() {
    return detail::Logger(detail::Logger::MessagePriority::warning);
}

detail::Logger error() {
    return detail::Logger(detail::Logger::MessagePriority::error);
}

detail::Logger fatal() {
    return detail::Logger(detail::Logger::MessagePriority::fatal);
}

} // namespace cacos::log
