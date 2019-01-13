#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace cacos {

class UnknownVariableName : public std::runtime_error {
public:
    UnknownVariableName(const std::string& name);
};

class InlineVariableParsingError : public std::runtime_error {
public:
    InlineVariableParsingError(const std::string& what);
};

enum class UnknownVariablePolicy {
    IGNORE,
    THROW
};

class InlineVariables {
public:
    InlineVariables(UnknownVariablePolicy policy = UnknownVariablePolicy::IGNORE)
        : policy_(policy)
    {}

    void set(const std::string& key, const std::string& value);

    std::string parse(std::string_view str) const;

private:
    UnknownVariablePolicy policy_;
    std::unordered_map<std::string, std::string> vars_;
};

} // namespace casos
