#pragma once

#include <string_view>
#include <optional>

namespace cacos::config::defaults {

std::optional<std::string_view> find(std::string_view key);

}
