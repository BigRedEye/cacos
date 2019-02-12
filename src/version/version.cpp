#include "cacos/version/version.h"
#include "cacos/version/git.h"
#include "cacos/version/id.h"

#include <fmt/format.h>

namespace cacos::commands {

int version(int, const char*[]) {
    fmt::print("cacos version {}\nbuild {}\n", CACOS_VERSION, version::build::timestamp());
    if (version::git::hasGitState()) {
        fmt::print("git commit {}", version::git::commit());
        if (version::git::dirty()) {
            fmt::print(", has unstaged changes");
        }
        fmt::print("\n");
    }
    return 0;
}

} // namespace cacos::commands
