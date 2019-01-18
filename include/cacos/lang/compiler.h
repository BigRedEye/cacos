#pragma once

#include "cacos/lang/translator.h"

namespace cacos::lang {

class Compiler : public Translator {
public:
    Compiler(const cpptoml::table& t, const fs::path& binaryDir_);

    executable::Executable process(const fs::path& source) const override;

private:
    fs::path binaryDir_;
};

} // namespace cacos::lang
