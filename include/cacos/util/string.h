#pragma once

#include <sstream>
#include <string>
#include <string_view>

namespace cacos::util {

inline std::string str(std::string_view view) {
    return std::string(view.begin(), view.end());
}

template<typename T>
inline T from_string(std::string_view s) {
    static std::istringstream ss;
    ss.exceptions(std::istringstream::failbit);
    ss.clear();
    ss.str(str(s));
    T result;
    ss >> result;
    return result;
}

template<typename T>
inline std::string to_string(T&& t) {
    static std::ostringstream os;
    os.clear();
    os.str("");
    os << t;
    return os.str();
}

inline bool starts_with(std::string_view str, std::string_view prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

inline bool ends_with(std::string_view str, std::string_view suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), std::string_view::npos, suffix) == 0;
}

template<typename... Args>
inline std::string join(Args&&... args) {
    return ("" + ... + to_string(args));
}

} // namespace cacos