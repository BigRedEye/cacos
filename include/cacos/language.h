#pragma once

#include "util.h"
#include "executable.h"

#include <cpptoml.h>

#include <optional>

namespace cacos {

class Language {
public:
    static Language parse(const cpptoml::table& config);

    Executable createExecutable(const fs::path& source) const;

private:
    std::optional<Executable> compiler_;
    std::optional<Executable> interpreter_;
};

}
