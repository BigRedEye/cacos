#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

#include "cacos/util/string.h"

namespace cacos {

class UnknownVariableName : public std::runtime_error {
public:
    UnknownVariableName(const std::string& name);
};

class InlineVariableParsingError : public std::runtime_error {
public:
    InlineVariableParsingError(const std::string& what);
};

enum class UnknownVariablePolicy { IGNORE, THROW };

class InlineVariables {
public:
    InlineVariables(
        std::string_view prefix,
        UnknownVariablePolicy policy = UnknownVariablePolicy::IGNORE)
        : prefix_(util::str(prefix))
        , policy_(policy) {
    }

    InlineVariables(UnknownVariablePolicy policy = UnknownVariablePolicy::IGNORE)
        : InlineVariables(DEFAULT_PREFIX, policy) {
    }

    void set(const std::string& key, const std::string& value);

    std::string parse(std::string_view str) const;

private:
    static constexpr std::string_view DEFAULT_PREFIX = "@";

    std::string prefix_;
    UnknownVariablePolicy policy_;
    std::unordered_map<std::string, std::string> vars_;
};

} // namespace cacos
