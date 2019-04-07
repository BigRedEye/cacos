#include "cacos/executable/flags.h"

#include "cacos/util/string.h"

namespace cacos::executable {

Flags::Flags(const std::vector<std::string>& flags)
    : flags_(flags) {
}

bool Flags::operator==(const Flags& other) const {
    return flags_ == other.flags_;
}

bool Flags::operator!=(const Flags& other) const {
    return !operator==(other);
}

std::vector<std::string> Flags::build(const InlineVariables& vars) const {
    std::vector<std::string> result;
    for (auto&& f : flags_) {
        result.push_back(vars.parse(f));
    }
    return result;
}

void Flags::prepend(const Flags& flags) {
    flags_.insert(flags_.begin(), flags.flags_.begin(), flags.flags_.end());
}

void Flags::append(const Flags& flags) {
    flags_.reserve(flags.flags_.size() + flags_.size());
    flags_.insert(flags_.end(), flags.flags_.begin(), flags.flags_.end());
}

} // namespace cacos::executable
