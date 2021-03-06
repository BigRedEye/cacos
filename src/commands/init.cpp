#include "cacos/commands/init.h"
#include "cacos/config/config.h"
#include "cacos/config/default.h"

#include "cacos/util/logger.h"
#include "cacos/util/string.h"

#include <cpparg/cpparg.h>

#include <fstream>
#include <string>

namespace cacos::commands {

int init(int argc, const char* argv[]) {
    fs::path dir;

    cpparg::parser parser("cacos init");
    parser.title("Initialize new workspace");

    fs::path workspace = fs::current_path();

    std::optional<std::string> task;
    // clang-format off
    parser
        .add("task")
        .optional()
        .description("Ejudge task name (sm07-5, kr03-2")
        .value_type("TASK")
        .handle([&](std::string_view sv) {
            task = util::str(sv);
        });
    // clang-format on

    parser.add_help('h', "help");

    parser.parse(argc, argv);

    std::vector<std::string_view> required_dirs = {"test"};

    for (auto dir : required_dirs) {
        if (!fs::create_directories(workspace / dir)) {
            throw std::runtime_error("Cannot create directory " + (workspace / dir).string());
        }
    }

    util::file::write(workspace / "cacos.toml", config::defaults::find("task").value());

    return 0;
}

} // namespace cacos::commands
