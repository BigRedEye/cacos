#pragma once

#include <optional>
#include <string_view>

namespace cacos::config::defaults {

std::optional<std::string_view> find(std::string_view key);

} // namespace cacos::config::defaults
