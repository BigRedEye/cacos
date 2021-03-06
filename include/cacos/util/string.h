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

namespace string {

namespace detail {

template<typename T>
using istream_read_t = decltype(std::declval<std::istream&>() >> std::declval<T&>());

template<typename T, typename U = istream_read_t<T>>
std::true_type test(T);

std::true_type test(std::string);
std::true_type test(std::string_view);

template<typename... Args>
std::false_type test(Args...);

template<typename T>
struct is_convertible_from_string : decltype(test(std::declval<T>())) {};

template<>
struct is_convertible_from_string<void> : std::false_type {};

template<typename T>
inline constexpr bool is_convertible_from_string_v = is_convertible_from_string<T>::value;

} // namespace detail

class FromStringError : public std::runtime_error {
public:
    FromStringError()
        : std::runtime_error("Cannot parse from string") {
    }
};

template<typename T>
inline T from(std::string_view s) {
    static_assert(
        detail::is_convertible_from_string_v<T>,
        "Cannot find std::istream& operator>>(std::istream&, T&)");

    if constexpr (std::is_same_v<std::string, T>) {
        return str(s);
    } else if constexpr (std::is_same_v<std::string_view, T>) {
        return s;
    } else {
        static std::istringstream ss;
        ss.clear();
        ss.str(str(s));
        T result;
        ss >> result;

        if (!ss) {
            throw FromStringError{};
        }

        if (ss.peek() != std::char_traits<char>::eof()) {
            throw FromStringError{};
        }

        return result;
    }
}

template<typename T>
inline std::string to(T&& t) {
    static std::ostringstream os;
    os.clear();
    os.str("");
    os << t;
    return os.str();
}

inline bool starts(std::string_view str, std::string_view prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

inline bool ends(std::string_view str, std::string_view suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), std::string_view::npos, suffix) == 0;
}

template<typename... Args>
inline std::string join(Args&&... args) {
    return ("" + ... + to(args));
}

template<typename T, typename A, typename U>
inline std::string join(const std::vector<T, A>& ts, U delim) {
    if (ts.empty()) {
        return {};
    }
    std::string result = to(ts[0]);
    for (auto&& t : skip(ts, 1)) {
        result += join(delim, t);
    }
    return result;
}

std::string escape(const std::string& s);
std::string escape(std::string_view s);

} // namespace string

namespace file {

inline constexpr size_t defaultBufferCapacity = 4096;

inline std::string read(std::ifstream& in) {
    std::string result;
    std::string buffer(defaultBufferCapacity, '\0');
    do {
        in.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        auto bytes = static_cast<size_t>(in.gcount());
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

} // namespace file

} // namespace cacos::util
