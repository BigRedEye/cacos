#include "cacos/task/opts.h"

namespace cacos::opts {

std::string_view serialize(ArchBits bits) {
    switch (bits) {
    case ArchBits::x32:
        return "32";
    case ArchBits::x64:
        return "64";
    default:
        return "undefined";
    }
}

} // namespace cacos::opts
