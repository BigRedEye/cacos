#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace cacos::executable {

class Placeholder {
public:
    Placeholder(std::string key);

    const std::string& key() const;
private:
    std::string key_;
};

class Flags {
public:
    Flags(const std::vector<std::string>& flags);

    std::vector<std::string> build(const std::unordered_map<std::string, std::string>& vars);

private:
    std::vector<std::variant<Placeholder, std::string>> flags_;
};

}
