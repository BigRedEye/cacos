#pragma once

#include "cacos/executable/executable.h"

#include <cpptoml.h>

#include <string_view>

namespace cacos {

class Config {
public:

    Config(const fs::path& ws, std::string_view filename = DEFAULT_FILENAME);

    static fs::path defaultConfig();
    static constexpr std::string_view DEFAULT_FILENAME = "cacos.toml";

private:
    fs::path main;
};

std::optional<toml::toml> find_config(const std::vector<std::string>& paths, std::string_view filename);

}
