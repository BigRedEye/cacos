#include "cacos/config/config.h"

#include <cpptoml.h>

#include "cacos/util/logger.h"

#include <cstdlib>
#include <fstream>
#include <string_view>

#ifdef CACOS_OS_UNIX
#include <pwd.h>
#include <sys/types.h>
#endif

namespace cacos::config {

namespace {
fs::path defaultDir() {
    return fs::path(CACOS_CONFIG_PREFIX) / "cacos";
}

fs::path homeDir() {
#ifdef CACOS_OS_UNIX
    struct passwd* pass = getpwuid(getuid());
    if (pass && pass->pw_dir) {
        return fs::path(pass->pw_dir);
    }
#endif

    static const char* vars[] = {"HOME", "HOMEPATH", "HOMESHARE", "USERPROFILE", "HOMEDRIVE"};
    for (auto var : vars) {
        const char* path;
        if ((path = getenv(var))) {
            return fs::path(path);
        }
    }
    Logger::warning() << "Unable to determine user home dir";
    return fs::current_path();
}

fs::path userConfigDir() {
    fs::path path = homeDir() / ".config" / "cacos";
    if (!fs::exists(path)) {
        fs::create_directories(path);
    }
    return path;
}

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

std::optional<fs::path> findConfigFile(const std::vector<fs::path>& alternatives) {
    for (auto&& path : alternatives) {
        if (fs::exists(path)) {
            return path;
        }
    }

    return {};
}

std::optional<fs::path> findConfig(
    const fs::path& overriden,
    const fs::path& config,
    const std::vector<fs::path>& defaults) {
    auto firstDefault = findConfigFile(defaults);
    if (!fs::exists(config) && firstDefault) {
        try {
            fs::create_directories(config.parent_path());
        } catch (...) {
        }
        fs::copy(*firstDefault, config);
    }

    return findConfigFile({overriden, config});
}

} // namespace

ConfigError::ConfigError(const std::string& what)
    : std::runtime_error(what) {
}

BadWorkspace::BadWorkspace(const std::string& what)
    : std::runtime_error("Not a cacos workspace " + what) {
}

fs::path Config::dir(DirType type) const {
    fs::path result;
    switch (type) {
    case DirType::workspace:
        result = workspace();
        break;
    case DirType::binary:
        result = fs::temp_directory_path() / "cacos";
        break;
    case DirType::test:
        result = workspace() / "test";
        break;
    case DirType::config:
        result = userConfigDir();
        break;
    case DirType::cache:
        result = workspace() / ".cacos" / "cache";
        break;
    case DirType::task:
        result = workspace() / ".cacos";
        break;
    default:
        break;
    }

    return createIfNotExists(result);
}

fs::path Config::file(FileType type) const {
    switch (type) {
    case FileType::config:
        return dir(DirType::config) / CONFIG_FILE;
    case FileType::langs:
        return dir(DirType::config) / LANGS_FILE;
    case FileType::token:
        return dir(DirType::config) / TOKEN_FILE;
    case FileType::cookies:
        return dir(DirType::config) / COOKIES_FILE;
    case FileType::taskConfig:
        return dir(DirType::task) / CONFIG_FILE;
    default:
        return {};
    }
}

fs::path Config::workspace() const {
    if (!fs::exists(workspace_ / ".cacos")) {
        throw BadWorkspace();
    }
    return workspace_;
}

Config::Config()
    : workspace_(fs::current_path()) {
    fs::path config;
    try {
        config = findConfig("", file(FileType::config), {defaultDir() / CONFIG_FILE}).value();
    } catch (...) {
        std::throw_with_nested(
            ConfigError("Cannot find main config file; you may consider reinstall the program."));
    }

    globalConfig_ = cpptoml::parse_file(config);

    try {
        fs::path taskConfig = file(FileType::taskConfig);
        if (fs::exists(taskConfig)) {
            taskConfig_ = cpptoml::parse_file(taskConfig);
        }
    } catch (const BadWorkspace&) {
        taskConfig_.reset();
    }
}

Config::Config(cpparg::parser& parser, ui64 mask)
    : Config() {
    // clang-format off
    if (mask & LANGS) {
        parser
            .add("langs")
            .optional()
            .description("Langs file")
            .default_value(file(FileType::langs))
            .value_type("FILE")
            .handle([this](std::string_view path) {
                fs::path langs;
                try {
                    langs = findConfig(
                        path,
                        dir(DirType::config) / LANGS_FILE,
                        {defaultDir() / LANGS_FILE}).value();
                } catch (...) {
                    std::throw_with_nested(
                        ConfigError("Cannot find langs file")
                    );
                }

                parseLangs(langs);
            });
    }

    if (mask & EJUDGE) {
        parser
            .add("login")
            .optional()
            .description("Ejudge login")
            .default_value("config{ejudge.login}")
            .handle([&](auto sv) {
                ejudge_.login.login = argOrConfig<std::string>(util::str(sv), "ejudge.login");
            });

        parser
            .add("password")
            .optional()
            .description("Ejudge password")
            .default_value("config{ejudge.password}")
            .handle([&, this](auto sv) {
                ejudge_.login.password = argOrConfig<std::string>(util::str(sv), "ejudge.password");
            });

        parser
            .add("cookie")
            .optional()
            .description("Ejudge cookie (EJSID=...)")
            .default_value("config{ejudge.cookie}")
            .handle([&](auto sv) {
                ejudge_.session.ejsid = argOrConfig<std::string>(util::str(sv), "ejudge.cookie");
            });

        parser
            .add("token")
            .optional()
            .description("Ejudge session token from url")
            .default_value("config{ejudge.token}")
            .handle([&](auto sv) {
                ejudge_.session.token = argOrConfig<std::string>(util::str(sv), "ejudge.token");
            });

        parser
            .add("contest_id")
            .optional()
            .description("Ejudge contest_id")
            .default_value("config{ejudge.contest_id}")
            .handle([&](auto sv) {
                ejudge_.contestId = argOrConfig<i32>(util::str(sv), "ejudge.contest_id", ConfigError("Invalid contest_id"));
            });

        parser
            .add("url")
            .optional()
            .description("Ejudge login page")
            .default_value("config{ejudge.url}")
            .handle([&](auto url) {
                ejudge_.url = argOrConfig<std::string>(util::str(url), "ejudge.url", ConfigError("Invalid ejudge url"));
            });
    }

    parser
        .add('v')
        .optional()
        .repeatable()
        .description("Increase verbosity level")
        .no_argument()
        .handle([] (auto) {
            Logger::increaseVerbosity();
        });

    parser
        .add_help('h', "help");
    // clang-format on
}

void Config::setImpl(
    std::string_view path,
    const std::shared_ptr<cpptoml::base>& value,
    ConfigType type) const {
    std::shared_ptr<cpptoml::table> cfg =
        (type == ConfigType::global ? globalConfig_ : taskConfig_);
    if (!cfg) {
        throw BadWorkspace();
    }

    std::vector<std::string_view> tables = util::split(path, ".");

    std::string_view key = tables.back();
    tables.pop_back();

    for (auto keyView : tables) {
        std::string key = util::str(keyView);
        if (!cfg->contains(key)) {
            cfg->insert(key, cpptoml::make_table());
        }
        auto node = cfg->get_table(key);
        cfg = node;
    }

    cfg->insert(util::str(key), value);
}

void Config::parseLangs(const fs::path& langs) {
    auto table = cpptoml::parse_file(langs);

    if (table) {
        langs_.langs = lang::LanguageTable(*table, dir(DirType::binary));
    } else {
        throw ConfigError("Cannot parse langs file");
    }
}

const lang::LanguageTable& Config::langs() const {
    return langs_.langs;
}

const opts::EjudgeOpts& Config::ejudge() const {
    return ejudge_;
}

void Config::dump(ConfigType type) const {
    auto& cfg = (type == ConfigType::global ? globalConfig_ : taskConfig_);
    if (!cfg) {
        throw ConfigError("Cannot save config");
    }

    std::ofstream out(
        dir(type == ConfigType::global ? DirType::config : DirType::task) / CONFIG_FILE);
    out << *cfg;
}

} // namespace cacos::config
