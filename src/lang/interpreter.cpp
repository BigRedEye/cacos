#include "cacos/lang/compiler.h"

#include "cacos/config/config.h"

#include "cacos/util/inline_variables.h"

namespace cacos::lang {

Interpreter::Interpreter(const cpptoml::table& t)
    : Translator(t) {
}

executable::Executable Interpreter::process(const fs::path& source) const {
    InlineVariables vars;

    vars.set("source", source);

    executable::Flags flags = common_;
    flags.append(debug_);

    return executable::Executable(exe_.path(), flags.build(vars));
}

} // namespace cacos::lang
