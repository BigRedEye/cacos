#pragma once

#include "cacos/util/util.h"

#include "cacos/process/limits.h"

#include <string_view>
#include <vector>

namespace cacos::opts {

enum class ArchBits {
    x32,
    x64,
    undefined,
};

std::string_view serialize(ArchBits bits);
ArchBits parseArchBits(std::string_view);

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
    undefined,
};

std::string_view serialize(BuildType type);
BuildType parseBuildType(std::string_view);

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
    process::Limits limits;
};

} // namespace cacos::opts
