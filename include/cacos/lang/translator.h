#pragma once

#include "cacos/executable/executable.h"

#include "cpptoml.h"

namespace cacos::lang {

class Translator {
public:
    Translator(const cpptoml::table& table);

    bool operator==(const Translator& other) const;
    bool operator!=(const Translator& other) const;

protected:
    executable::Executable exe_;
    executable::Flags common_;
    executable::Flags debug_;
    executable::Flags release_;
};

} // namespace cacos::lang
