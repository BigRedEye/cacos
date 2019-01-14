#pragma once

#include "cacos/lang/translator.h"

namespace cacos::lang {

class Interpreter : public Translator {
public:
    Interpreter(const cpptoml::table& t);

    executable::Executable process(const fs::path& source) override;
};

} // namespace cacos::lang
