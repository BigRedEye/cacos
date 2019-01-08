#include "cacos/common_args.h"

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
            .description("Config file")
            .value_type("CFG")
            .default_value("cacos.toml")
            .store(opts.config);
    }

    parser
        .add_help('h', "help");

    return parser;
}

}
