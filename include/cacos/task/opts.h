#pragma once

#include "cacos/util/util.h"

#include <string_view>
#include <vector>

namespace cacos::opts {

enum class ArchBits {
    x32,
    x64,
    undefined,
};

std::string_view serialize(ArchBits bits);

inline constexpr ArchBits hostArch() {
    if constexpr (sizeof(void*) == 8) {
        return ArchBits::x64;
    }
    if constexpr (sizeof(void*) == 4) {
        return ArchBits::x32;
    }
    return ArchBits::undefined;
}

enum class BuildType {
    debug,
    release,
};

struct CompilerOpts {
    ArchBits archBits = hostArch();
    BuildType buildType = BuildType::debug;
};

struct ExeOpts {
    std::vector<fs::path> sources;
    CompilerOpts compiler;
};

struct TaskOpts {
    ExeOpts exe;
};

}
