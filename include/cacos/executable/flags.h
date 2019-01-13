#pragma once

#include "cacos/util/inline_variables.h"

#include <string>
#include <string_view>
#include <vector>

namespace cacos::executable {

class Flags {
public:
    Flags(const std::vector<std::string>& flags);

    std::vector<std::string> build(const InlineVariables& vars) const;

private:
    std::vector<std::string> flags_;
};

}
