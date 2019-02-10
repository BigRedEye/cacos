#include "cacos/util/string.h"

#include <fmt/format.h>

#include <ctype.h>

namespace cacos::util::string {

std::string escape(const std::string& s) {
    return escape(std::string_view{s});
}

std::string escape(std::string_view s) {
    std::string result;
    result.reserve(s.size());

    for (char c : s) {
        if (std::isprint(c)) {
            result.push_back(c);
        } else if (std::isspace(c)) {
            /* https://en.cppreference.com/w/cpp/language/escape */
            /* https://en.cppreference.com/w/cpp/string/byte/isspace */
            switch (c) {
            case '\f':
                result += "\\f";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            case '\v':
                result += "\\v";
                break;
            case ' ':
            case '\n':
            default:
                result.push_back(c);
            }
        } else {
            result += fmt::format("\\x{:02x}", static_cast<unsigned char>(c));
        }
    }

    return result;
}

} // namespace cacos::util::string
