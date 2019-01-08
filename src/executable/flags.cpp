#include "cacos/executable/flags.h"

namespace cacos::executable {

static constexpr char PLACEHOLDER_PREFIX = '@';

Placeholder::Placeholder(std::string key)
    : key_(std::move(key))
{
    if (key_.empty() || key_[0] != PLACEHOLDER_PREFIX) {
        throw std::runtime_error("Placeholder name should start with " + std::string(1, PLACEHOLDER_PREFIX));
    }
}

const std::string& Placeholder::key() const {
    return key_;
}


class UnknownKeyError : public std::runtime_error {
public:
    UnknownKeyError(const std::string& s)
        : std::runtime_error("No variable given for key " + s) {
    }
};


Flags::Flags(const std::vector<std::string>& flags) {
    flags_.reserve(flags.size());
    for (auto&& s : flags) {
        if (s.size() >= 1 && s[0] == PLACEHOLDER_PREFIX) {
            flags_.push_back(Placeholder(s));
        } else {
            flags_.push_back(s);
        }
    }
}

std::vector<std::string> Flags::build(const std::unordered_map<std::string, std::string>& vars) {
    std::vector<std::string> result;
    for (auto& f : std::as_const(flags_)) {
        std::visit([&](auto&& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<Placeholder, T>) {
                std::string key = value.key();
                auto it = vars.find(key);
                if (it == vars.end()) {
                    throw UnknownKeyError(std::string(key.begin(), key.end()));
                } else {
                    result.push_back(it->second);
                }
            } else {
                result.push_back(value);
            }
        }, f);
    }
    return result;
}

} // namespace cacos::executable
