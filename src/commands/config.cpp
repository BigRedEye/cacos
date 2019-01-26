#include "cacos/commands/config.h"
#include "cacos/config/config.h"

#include <cpparg/cpparg.h>

namespace cacos::commands {

int config(int argc, const char* argv[]) {
    cpparg::parser parser("cacos config");
    parser.title("Manage config");

    config::Config cfg;

    // clang-format off
    parser
        .add("login")
        .optional()
        .description("Ejudge login")
        .handle([&](auto sv) {
            cfg.set("ejudge.login", util::str(sv));
        });

    parser
        .add("password")
        .optional()
        .description("Ejudge password")
        .handle([&](auto sv) {
            cfg.set("ejudge.password", util::str(sv));
        });

    parser
        .add("contest_id")
        .optional()
        .description("Ejudge contest_id")
        .handle([&](auto sv) {
            cfg.set("ejudge.contest_id", util::from_string<i32>(sv));
        });

    parser
        .add("url")
        .optional()
        .description("Ejudge login page")
        .handle([&](auto sv) {
            cfg.set("ejudge.url", util::str(sv));
        });
    // clang-format on

    parser.add_help('h', "help");
    parser.parse(argc, argv);

    cfg.dump(config::ConfigType::global);

    return 0;
}

} // namespace cacos::commands
