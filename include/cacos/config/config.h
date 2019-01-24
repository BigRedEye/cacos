#pragma once

#include "cacos/executable/executable.h"
#include "cacos/options.h"

#include "cacos/lang/opts.h"
#include "cacos/ejudge/opts.h"

#include <cpparg/cpparg.h>

#include <cpptoml.h>

#include <string_view>
#include <optional>

namespace cacos::config {

class ConfigError : public std::runtime_error {
public:
    ConfigError(const std::string& err);
};

class BadWorkspace : public std::runtime_error {
public:
    BadWorkspace(const std::string& err = "");
};

enum class DirType {
    workspace,
    binary,
    test,
    cache,
    config,
    task,
};

enum class ConfigType {
    global,
    task,
};

enum Mask : ui64 {
    LANGS     = ui64{1} << 1,
    EJUDGE    = ui64{1} << 2,
    ALL       = ~ui64{0},
};

class Config {
public:
    Config();
    Config(cpparg::parser& parser, ui64 opts);

    fs::path dir(DirType type) const;
    fs::path workspace() const;

    const lang::LanguageTable& langs() const;
    const opts::EjudgeOpts& ejudge() const;

    template<typename T>
    void set(const std::string& key, T&& value, ConfigType type = ConfigType::global) {
        auto& cfg = (type == ConfigType::global ? globalConfig_ : taskConfig_);
        if (!cfg) {
            throw BadWorkspace();
        }
        cfg->insert(key, std::forward<T>(value));
    }

    void dump(ConfigType type) const;

private:
    void parseLangs(const fs::path& langs);

    template<typename T>
    std::optional<T> argOrConfig(const std::string& arg, const std::string& key) {
        if (arg == util::join("config{", key, "}")) {
            auto value = std::optional<T>{};
            if (taskConfig_) {
                auto res = taskConfig_->get_qualified_as<T>(key);
                if (res) {
                    value = *res;
                }
            }
            if (!value) {
                auto res = globalConfig_->get_qualified_as<T>(key);
                if (res) {
                    value = *res;
                }
            }
            return value;
        } else {
            return util::from_string<T>(arg);
        }
    }

    template<typename T, typename E>
    T argOrConfig(
        const std::string& arg,
        const std::string& key,
        E&& exception) {
        try {
            return argOrConfig<T>(arg, key).value();
        } catch (const std::bad_variant_access&) {
            std::throw_with_nested(std::forward<E>(exception));
        }
    }

    static constexpr std::string_view CONFIG_FILE = "cacos.toml";
    static constexpr std::string_view LANGS_FILE = "langs.toml";

private:
    fs::path workspace_;
    std::shared_ptr<cpptoml::table> globalConfig_;
    std::shared_ptr<cpptoml::table> taskConfig_;
    opts::LangOpts langs_;
    opts::EjudgeOpts ejudge_;
};

}
