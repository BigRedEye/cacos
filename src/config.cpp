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

fs::path createIfNotExists(const fs::path& path) {
    if (!fs::is_directory(path)) {
        std::error_code errc;
        fs::create_directories(path, errc);
        if (errc) {
            throw ConfigError(errc.message());
        }
    }
    return path;
}

}

fs::path Config::directory(DirectoryType type) const {
    fs::path result;
    switch (type) {
    case DirectoryType::workspace:
        result = workspace();
        break;
    case DirectoryType::binary:
        result = workspace() / "bin";
        break;
    case DirectoryType::test:
        result = workspace() / "test";
        break;
    case DirectoryType::cache:
        result = workspace() / "cacos";
        break;
    default:
        break;
    }

    return createIfNotExists(result);
}

namespace {

std::optional<fs::path> findConfigFile(const std::vector<fs::path>& alternatives) {
    for (auto&& path : alternatives) {
        if (fs::exists(path)) {
            return path;
        }
    }

    return {};
}

}

Config::Config(const Options& opts) {
    auto config = findConfigFile({ opts.config, userDir() / "cacos.toml", defaultDir() / "cacos.toml" });
    auto langs = findConfigFile({ opts.langs, userDir() / "langs.toml", defaultDir() / "langs.toml" });

    if (langs) {
        parseLangs(*langs);
    } else {
        throw ConfigError("Cannot find langs file");
    }
}

}
