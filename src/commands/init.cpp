#include "cacos/commands/init.h"
#include "cacos/config/config.h"
#include "cacos/options.h"

#include "cacos/util/logger.h"

#include <cpparg/cpparg.h>

#include <string>
#include <fstream>
#include <filesystem>

namespace cacos::commands {

int init(int argc, const char* argv[]) {
    fs::path dir;

    cpparg::parser parser("cacos new");
    parser.title("Initialize new workspace");

    fs::path workspace;

    parser
        .positional("workspace")
        .optional()
        .description("Path to workspace")
        .value_type("DIR")
        .default_value(".")
        .handle([&](std::string_view path) {
            workspace = path;
        });

    parser.parse(argc, argv);

    std::vector<std::string_view> required_dirs = { ".cacos", "test" };

    for (auto dir : required_dirs) {
        if (!fs::create_directories(workspace / dir)) {
            throw std::runtime_error("Cannot create directory");
        }
    }

    return 0;
}

}
