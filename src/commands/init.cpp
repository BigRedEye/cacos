#include "cacos/commands/init.h"
#include "cacos/config/config.h"
#include "cacos/config/default.h"

#include "cacos/util/logger.h"
#include "cacos/util/string.h"

#include <cpparg/cpparg.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace cacos::commands {

int init(int argc, const char* argv[]) {
    fs::path dir;

    cpparg::parser parser("cacos new");
    parser.title("Initialize new workspace");

    fs::path workspace;

    // clang-format off
    parser
        .positional("workspace")
        .optional()
        .description("Path to workspace")
        .value_type("DIR")
        .default_value(".")
        .handle([&](std::string_view path) {
            workspace = path;
        });
    // clang-format on

    parser.parse(argc, argv);

    std::vector<std::string_view> required_dirs = {".cacos", "test"};

    for (auto dir : required_dirs) {
        if (!fs::create_directories(workspace / dir)) {
            throw std::runtime_error("Cannot create directory");
        }
    }

    util::file::write(workspace / "cacos.toml", config::defaults::find("task").value());

    return 0;
}

} // namespace cacos::commands
