#include "cacos/commands/config.h"
#include "cacos/config/config.h"

#include <cpparg/cpparg.h>

namespace cacos::commands {

int global(int argc, const char* argv[]) {
    cpparg::parser parser("cacos config global");
    parser.title("Manage global config");

    config::Config cfg;

    // clang-format off
    parser
        .add("login")
        .optional()
        .value_type("LOGIN")
        .description("Ejudge login")
        .handle([&](auto sv) {
            cfg.set("ejudge.login", util::str(sv));
        });

    parser
        .add("password")
        .optional()
        .value_type("PASSWORD")
        .description("Ejudge password")
        .handle([&](auto sv) {
            cfg.set("ejudge.password", util::str(sv));
        });

    parser
        .add("contest_id")
        .optional()
        .value_type("INT")
        .description("Ejudge contest_id")
        .handle([&](auto sv) {
            cfg.set("ejudge.contest_id", util::string::from<i32>(sv));
        });

    parser
        .add("url")
        .optional()
        .value_type("URL")
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

int task(int argc, const char* argv[]) {
    cpparg::parser parser("cacos config task");
    parser.title("Manage task config");

    config::Config cfg;

    // clang-format off
    parser
        .add("sources")
        .optional()
        .value_type("FILES")
        .description("Main executable sources, separated with ',' or ' '")
        .handle([&](auto sv) {
            auto sources = util::split<std::string>(sv, ", ");
            auto array = cpptoml::make_array();
            for (auto&& src : sources) {
                array->push_back(std::move(src));
            }
            cfg.setBase("exe.sources", array, config::ConfigType::task);
        });

    parser
        .add("arch")
        .optional()
        .value_type("ARCH")
        .description("Main executable architecture")
        .handle([&](auto arch) {
            cfg.set("exe.arch", util::str(arch), config::ConfigType::task);
        });

    parser
        .add("build")
        .optional()
        .value_type("BUILD_TYPE")
        .description("Default build type, 'debug' or 'release'")
        .handle([&](auto type) {
            cfg.set("exe.build", util::str(type), config::ConfigType::task);
        });

    parser
        .add("tl")
        .optional()
        .value_type("SECONDS")
        .description("Time limit")
        .handle([&](auto tl) {
            cfg.set("limits.time", util::str(tl), config::ConfigType::task);
        });

    parser
        .add("ml")
        .optional()
        .value_type("MiB")
        .description("Memory limit")
        .handle([&](auto ml) {
            cfg.set("limits.memory", util::str(ml), config::ConfigType::task);
        });
    // clang-format on

    parser.add_help('h', "help");
    parser.parse(argc, argv);

    cfg.dump(config::ConfigType::task);

    return 0;
}

int config(int argc, const char* argv[]) {
    cpparg::command_parser parser("cacos config");
    parser.title("Manage configs");

    // clang-format off
    parser
        .command("global")
        .description("Manage global config")
        .handle(global);

    parser
        .command("task")
        .description("Manage task config")
        .handle(task);
    // clang-format on

    return parser.parse(argc, argv);
}

} // namespace cacos::commands
