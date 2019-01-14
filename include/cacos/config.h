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

class Config {
public:
    Config(const Options& opts);

    fs::path workspace() const;

    const lang::LanguageTable& langs() const;

    static fs::path defaultDir();
    static fs::path userDir();
    fs::path binaryDir();

private:
    fs::path main_;
    lang::LanguageTable langs_;
};

}
