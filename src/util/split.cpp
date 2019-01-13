#include "cacos/util/split.h"

#include <algorithm>
#include <string_view>
#include <vector>

namespace cacos::util {

std::vector<std::string_view> split(std::string_view s, std::string_view delim) {
    std::vector<std::string_view> result;
    size_t left = 0;
    size_t right = 0;

    while ((right = s.find_first_of(delim, left)) != std::string_view::npos) {
        result.push_back(s.substr(left, right - left));
        left = right + 1;
    }
    result.push_back(s.substr(left, s.size() - left));
    return result;
}

} // namespace cacos::util
