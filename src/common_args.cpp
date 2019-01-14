#include "cacos/common_args.h"

#include "cacos/default_config.h"

namespace cacos {

cpparg::parser& setCommonOptions(cpparg::parser& parser, Options& opts, options::Mask mask) {
    if (mask & options::WORKSPACE) {
        parser
            .positional("workspace")
            .optional()
            .description("Path to workspace")
            .value_type("DIR")
            .default_value(".")
            .handle([&](std::string_view path) {
                opts.workspace = path;
            });
    }

    if (mask & options::CONFIG) {
        parser
            .add('c', "config")
            .optional()
            .description("Config file relative to the workspace directory")
            .value_type("FILE")
            .default_value(config::defaultDir() / "cacos.toml")
            .handler([&](std::string_view path) {
                opts.config = path;
            });
    }

    if (mask & options::CONFIG) {
        parser
            .add('l', "langs")
            .optional()
            .description("Langs file relative to the workspace directory")
            .value_type("FILE")
            .default_value(config::defaultDir() / "langs.toml")
            .handler([&](std::string_view path) {
                opts.langs = path;
            });
    }

    parser
        .add_help('h', "help");

    return parser;
}

}
