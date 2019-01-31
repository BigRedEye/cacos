#include "cacos/config/config.h"
#include "cacos/config/default.h"

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

fs::path homeDir() {
#ifdef CACOS_OS_UNIX
    struct passwd* pass = getpwuid(getuid());
    if (pass && pass->pw_dir) {
        return fs::path(pass->pw_dir);
    }
#endif
	
    static const char* vars[] = {"HOME", "LOCALAPPDATA", "USERPROFILE"};
    for (auto var : vars) {
		if (const char* str = getenv(var); str) {
			return str;
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

std::optional<fs::path>
    findConfig(const fs::path& overriden, const fs::path& config, std::string_view defaultKey) {
    auto defaultConfig = defaults::find(defaultKey);
    if (!fs::exists(config) && defaultConfig) {
        try {
            fs::create_directories(config.parent_path());
        } catch (...) {
        }
        std::ofstream ofs(config);
        ofs.write(defaultConfig->data(), static_cast<std::streamsize>(defaultConfig->size()));
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
    case DirType::temp:
        result = fs::temp_directory_path() / "cacos";
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
        return dir(DirType::temp) / TOKEN_FILE;
    case FileType::cookies:
        return dir(DirType::temp) / COOKIES_FILE;
    case FileType::taskConfig:
        return dir(DirType::workspace) / CONFIG_FILE;
    default:
        return {};
    }
}

fs::path Config::workspace() const {
    if (!fs::exists(workspace_ / "cacos.toml")) {
        throw BadWorkspace();
    }
    return workspace_;
}

Config::Config()
    : workspace_(fs::current_path()) {
    fs::path config;
    try {
        config = findConfig("", file(FileType::config), "config").value();
    } catch (...) {
        std::throw_with_nested(
            ConfigError("Cannot find main config file; you may consider reinstall the program."));
    }

    globalConfig_ = cpptoml::parse_file(config.string());

    try {
        fs::path taskConfig = file(FileType::taskConfig);
        if (fs::exists(taskConfig)) {
            taskConfig_ = cpptoml::parse_file(taskConfig.string());
        }
    } catch (const BadWorkspace&) {
        taskConfig_.reset();
    }

    parseConfig();
}

Config::Config(cpparg::parser& parser, ui64 mask)
    : Config() {
    // clang-format off
    if (mask & LANGS) {
        parser
            .add("langs")
            .optional()
            .description("Langs file")
            .value_type("FILE")
			.default_value(file(FileType::langs).string())
            .handle([this](std::string_view path) {
                fs::path langs;
				try {
					langs = findConfig(
						path,
						file(FileType::langs),
						"langs").value();
				} catch (const std::bad_optional_access&) {
					std::throw_with_nested(
						ConfigError("Bad optional access")
					);
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
            .handle([&](auto sv) {
                ejudge_.login.login = util::str(sv);
            });

        parser
            .add("password")
            .optional()
            .description("Ejudge password")
            .handle([&, this](auto sv) {
                ejudge_.login.password = util::str(sv);
            });

        parser
            .add("cookie")
            .optional()
            .description("Ejudge cookie (EJSID=...)")
            .handle([&](auto sv) {
                ejudge_.session.ejsid = util::str(sv);
            });

        parser
            .add("token")
            .optional()
            .description("Ejudge session token from url")
            .handle([&](auto sv) {
                ejudge_.session.token = util::str(sv);
            });

        parser
            .add("contest_id")
            .optional()
            .description("Ejudge contest_id")
            .handle([&](auto sv) {
                ejudge_.contestId = util::from_string<i32>(sv);
            });

        parser
            .add("url")
            .optional()
            .description("Ejudge login page")
            .handle([&](auto sv) {
                ejudge_.url = util::str(sv);
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
    auto table = cpptoml::parse_file(langs.string());

    if (table) {
        langs_.langs = lang::LanguageTable(*table, dir(DirType::binary));
    } else {
        throw ConfigError("Cannot parse langs file");
    }
}

void Config::parseConfig() {
    auto setIfHas = [](const auto& table, auto key, auto& value, auto dummy) {
        if (auto node =
                table->template get_qualified_as<std::decay_t<decltype(dummy)>>(util::str(key))) {
            value = *node;
        }
    };
    setIfHas(globalConfig_, "ejudge.login", ejudge_.login.login, std::string{});
    setIfHas(globalConfig_, "ejudge.password", ejudge_.login.password, std::string{});
    setIfHas(globalConfig_, "ejudge.cookie", ejudge_.session.ejsid, std::string{});
    setIfHas(globalConfig_, "ejudge.token", ejudge_.session.token, std::string{});
    setIfHas(globalConfig_, "ejudge.contest_id", ejudge_.contestId, i32{});
    setIfHas(globalConfig_, "ejudge.url", ejudge_.url, std::string{});

    if (taskConfig_) {
        if (auto node = taskConfig_->get_qualified_array_of<std::string>("exe.sources")) {
            std::transform(
                node->begin(), node->end(), std::back_inserter(task_.exe.sources), [](auto file) {
                    return fs::path(file);
                });
        }

        if (auto node = taskConfig_->get_qualified_as<std::string>("exe.arch")) {
            if (util::ends_with(*node, "32") || util::ends_with(*node, "86")) {
                task_.exe.compiler.archBits = opts::ArchBits::x32;
            } else if (util::ends_with(*node, "64")) {
                task_.exe.compiler.archBits = opts::ArchBits::x64;
            } else {
                task_.exe.compiler.archBits = opts::ArchBits::undefined;
            }
        }
    }
}

const lang::LanguageTable& Config::langs() const {
    return langs_.langs;
}

const opts::EjudgeOpts& Config::ejudge() const {
    return ejudge_;
}

const opts::TaskOpts& Config::task() const {
    return task_;
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
