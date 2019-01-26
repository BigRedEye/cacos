#pragma once

#include "cacos/util/inline_variables.h"

#include <string>
#include <string_view>
#include <vector>

namespace cacos::executable {

class Flags {
public:
    Flags() = default;
    Flags(const std::vector<std::string>& flags);

    std::vector<std::string> build(const InlineVariables& vars) const;

    void prepend(const Flags& flags);
    void append(const Flags& flags);

private:
    std::vector<std::string> flags_;
};

} // namespace cacos::executable
