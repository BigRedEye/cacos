#pragma once

#include "cacos/executable/executable.h"

#include "cacos/ejudge/opts.h"
#include "cacos/lang/opts.h"
#include "cacos/task/opts.h"

#include "cacos/util/split.h"
#include "cacos/util/temp_directory.h"

#include <cpparg/cpparg.h>

#include <cpptoml.h>

#include <optional>
#include <string_view>

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
    temp,
};

enum class FileType {
    config,
    langs,
    token,
    cookies,
    taskConfig,
};

enum class ConfigType {
    global,
    task,
};

enum Mask : ui64 {
    NONE = 0,
    LANGS = ui64{1} << 1,
    EJUDGE_SESSION = ui64{1} << 2,
    EJUDGE_LOGIN = ui64{1} << 3,
    TASK_EXE = ui64{1} << 4,
    KEEP_WORKING_DIRS = ui64{1} << 5,
    ALL = ~ui64{0},
};

class Config {
public:
    Config();
    Config(cpparg::parser& parser, ui64 opts = NONE);

    fs::path dir(DirType type) const;
    fs::path file(FileType type) const;
    fs::path workspace() const;

    const lang::LanguageTable& langs() const;
    const opts::EjudgeOpts& ejudge() const;
    const opts::TaskOpts& task() const;

    template<typename T>
    void set(std::string_view path, T&& value, ConfigType type = ConfigType::global) {
        setImpl(path, cpptoml::make_value(std::forward<T>(value)), type);
    }

    void setBase(
        std::string_view path,
        const std::shared_ptr<cpptoml::base>& value,
        ConfigType type = ConfigType::global) {
        setImpl(path, value, type);
    }

    void dump(ConfigType type) const;

private:
    void setImpl(
        std::string_view path,
        const std::shared_ptr<cpptoml::base>& value,
        ConfigType type) const;
    void parseLangs(const fs::path& langs);
    void parseConfig();

    static constexpr std::string_view CONFIG_FILE = "cacos.toml";
    static constexpr std::string_view LANGS_FILE = "langs.toml";
    static constexpr std::string_view TOKEN_FILE = "session.txt";
    static constexpr std::string_view COOKIES_FILE = "cookies.txt";

private:
    fs::path workspace_;
    util::TempDirectory cache_;

    std::shared_ptr<cpptoml::table> globalConfig_;
    std::shared_ptr<cpptoml::table> taskConfig_;

    opts::LangOpts langs_;
    opts::EjudgeOpts ejudge_;
    opts::TaskOpts task_;
};

} // namespace cacos::config
