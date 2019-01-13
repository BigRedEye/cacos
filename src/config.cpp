#include "cacos/config.h"

#include <cpptoml.h>

#include <fstream>
#include <string_view>

namespace cacos {

int config(int argc, const char* argv[]) {
    return !!argv[argc];
}

Config::Config(const fs::path& ws, std::string_view filename) {
    fs::path path = ws / filename;
    cpptoml::parse_file(path);
    std::shared_ptr<cpptoml::table> table;
    if (!fs::exists(path)) {
        table = cpptoml::parse_file(path);
    }
}

fs::path Config::defaultConfig() {
    return fs::path(DEFAULT_CONFIG_PREFIX) / "cacos" / "config.toml";
}

}
