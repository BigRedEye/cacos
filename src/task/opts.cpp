#include "cacos/task/opts.h"

#include "cacos/util/map.h"

#include <stdexcept>

namespace cacos::opts {

std::string_view serialize(ArchBits bits) {
    switch (bits) {
    case ArchBits::x32:
        return "32";
    case ArchBits::x64:
        return "64";
    default:
        throw std::runtime_error("Unknown architecture");
    }
}

BuildType parseBuildType(std::string_view sv) {
    // clang-format off
    return util::map<std::string_view>
        ("debug", BuildType::debug)
        ("release", BuildType::release)
        .map(sv, BuildType::undefined);
    // clang-format on
}

} // namespace cacos::opts
