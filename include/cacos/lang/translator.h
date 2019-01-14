#pragma once

#include "cacos/executable/executable.h"

#include "cpptoml.h"

namespace cacos::lang {

class Translator {
public:
    Translator(const cpptoml::table& table);
    virtual ~Translator() = default;

    virtual executable::Executable process(const fs::path& source) = 0;

protected:
    executable::Executable exe_;
    executable::Flags debug_;
    executable::Flags release_;
};

} // namespace cacos::lang
