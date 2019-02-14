#include "cacos/task/opts.h"

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

} // namespace cacos::opts
