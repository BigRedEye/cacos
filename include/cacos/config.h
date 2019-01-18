#pragma once

#include "cacos/executable/executable.h"

#include "cacos/options.h"

#include "cacos/lang/lang.h"

#include <cpptoml.h>

#include <string_view>


namespace cacos::config {

class ConfigError : public std::runtime_error {
public:
    ConfigError(const std::string& err);
};

enum class DirectoryType {
    workspace,
    binary,
    test,
    cache
};

class Config {
public:
    Config(const Options& opts);

    fs::path directory(DirectoryType type) const;
    fs::path workspace() const;

    const lang::LanguageTable& langs() const;

    static fs::path defaultDir();
    static fs::path userDir();

private:
    void parseLangs(const fs::path& langs);

private:
    fs::path workspace_;
    lang::LanguageTable langs_;
};

}
