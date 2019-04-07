#pragma once

#include "cacos/util/string.h"

#include <string_view>
#include <vector>

namespace cacos::util {

std::vector<std::string_view> split(std::string_view s, std::string_view delim);

template<typename T>
std::vector<T> split(std::string_view s, std::string_view delim) {
    auto views = split(s, delim);
    std::vector<T> result;

    result.reserve(views.size());
    for (auto&& view : views) {
        result.push_back(util::string::from<T>(view));
    }

    return result;
}

} // namespace cacos::util
