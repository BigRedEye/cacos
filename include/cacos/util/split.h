#pragma once

#include <string_view>
#include <vector>

namespace cacos::util {

std::vector<std::string_view> split(std::string_view s, std::string_view delim);

} // namespace cacos::util
