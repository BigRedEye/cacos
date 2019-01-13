#include "cacos/executable/flags.h"

#include "cacos/util/string.h"

namespace cacos::executable {

Flags::Flags(const std::vector<std::string>& flags)
    : flags_(flags) {
}

std::vector<std::string> Flags::build(const InlineVariables& vars) const {
    std::vector<std::string> result;
    for (auto&& f : flags_) {
        result.push_back(vars.parse(f));
    }
    return result;
}

} // namespace cacos::executable
