#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

#include "cacos/util/util.h"

namespace cacos::util {

inline std::string str(std::string_view view) {
    return std::string(view.begin(), view.end());
}

inline std::istringstream createISStream() {
    std::istringstream ss;
    ss.exceptions(std::istringstream::failbit);
    return ss;
}

template<typename T>
inline T from_string(std::string_view s) {
    static std::istringstream ss = createISStream();
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

template<typename T, typename A, typename U>
inline std::string join(const std::vector<T, A>& ts, U delim) {
    if (ts.empty()) {
        return {};
    }
    std::string result = to_string(ts[0]);
    for (auto&& t : skip(ts, 1)) {
        result += join(delim, t);
    }
    return result;
}

namespace file {

inline constexpr size_t defaultBufferCapacity = 4096;

inline std::string read(std::ifstream& in) {
    std::string result;
    std::string buffer(defaultBufferCapacity, '\0');
    do {
        in.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        size_t bytes = static_cast<size_t>(in.gcount());
        result.append(buffer.data(), bytes);
    } while (in);
    return result;
}

inline std::string read(const fs::path& path) {
    std::ifstream ifs(path);
    return read(ifs);
}

inline void write(const fs::path& path, std::string_view data) {
    std::ofstream ofs(path);
    ofs.write(data.data(), static_cast<std::streamsize>(data.size()));
}

}

} // namespace cacos::util
