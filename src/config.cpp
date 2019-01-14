#include "cacos/config.h"

#include <cpptoml.h>

#include "cacos/util/logger.h"

#include <cstdlib>
#include <fstream>
#include <string_view>

#ifdef CACOS_OS_UNIX
#include <sys/types.h>
#include <pwd.h>
#endif

namespace cacos::config {

ConfigError::ConfigError(const std::string& what)
    : std::runtime_error(what) {
}

fs::path Config::defaultDir() {
    return fs::path(CACOS_CONFIG_PREFIX) / "cacos";
}

namespace {
fs::path homeDir() {
#ifdef CACOS_OS_UNIX
    struct passwd* pass = getpwuid(getuid());
    if (pass && pass->pw_dir) {
        return fs::path(pass->pw_dir);
    }
#endif

    static const char* vars[] = { "HOME", "HOMEPATH", "HOMESHARE", "USERPROFILE", "HOMEDRIVE" };
    for (auto var : vars) {
        const char* path;
        if ((path = getenv(var))) {
            return fs::path(path);
        }
    }
    Logger::warning() << "Cannot determine user home dir";
    return fs::current_path();
}
}

fs::path Config::userDir() {
    return homeDir() / ".config";
}

namespace {

std::optional<fs::path> find(const std::vector<fs::path>& alternatives) {
    for (auto&& path : alternatives) {
        if (!fs::exists(path)) {
            return path;
        }
    }

    return {};
}

}

Config::Config(const Options& opts) {
    auto langs = find({ opts.langs, userDir() / "langs.toml", defaultDir() / "langs.toml" });
    auto config = find({ opts.langs, userDir() / "cacos.toml", defaultDir() / "cacos.toml" });

    if (!langs) {
        throw ConfigError("Cannot find langs file");
    }

    auto table = cpptoml::parse_file(*langs);

    if (table) {
        langs_ = lang::LanguageTable(*table);
    } else {
        throw ConfigError("Cannot parse langs file");
    }
}

}
