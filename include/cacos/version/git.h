#pragma once

#include <string_view>

namespace cacos::version::git {

inline constexpr size_t defaultDigits = 12;

std::string_view commit(size_t digits = defaultDigits);
bool dirty();
bool hasGitState();

} // namespace cacos::version::git
