#include "cacos/lang/compiler.h"

#include "cacos/config.h"

#include "cacos/util/inline_variables.h"

namespace cacos::lang {

Compiler::Compiler(const cpptoml::table& t, const fs::path& binaryDir)
    : Translator(t)
    , binaryDir_(binaryDir)
{}

executable::Executable Compiler::process(const fs::path& source) {
    InlineVariables vars;

    fs::path binary = binaryDir_ / source.filename();

    vars.set("source", source);
    vars.set("binary", binary);

    return executable::Executable(binary);
}

} // namespace cacos::lang
