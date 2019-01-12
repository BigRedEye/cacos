#include "cacos/executable/executable.h"

namespace cacos::executable {

Executable::Executable(const fs::path& exe)
    : Executable(exe, {})
{}

Executable::Executable(const fs::path& exe, const std::vector<std::string>& flags)
    : executable_(exe)
    , flags_(flags)
{}


}
