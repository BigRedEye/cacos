#include "cacos/common_args.h"

#include "cacos/config.h"
#ifdef qweqweqwe
namespace cacos {

cpparg::parser& setCommonOptions(cpparg::parser& parser, Options& opts, ui64 mask) {
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
            .default_value(config::Config::userConfigDir() / "cacos.toml")
            .handle([&](std::string_view path) {
                opts.config = path;
            });
    }

    if (mask & options::CONFIG) {
        parser
            .add('l', "langs")
            .optional()
            .description("Langs file relative to the workspace directory")
            .value_type("FILE")
            .default_value(config::Config::userConfigDir() / "langs.toml")
            .handle([&](std::string_view path) {
                opts.langs = path;
            });
    }

    if (mask & options::EJUDGE) {
        parser
            .add("password")
            .required()
            .description("Ejudge password")
            .default_value("config{ejudge.login}")
                .handle([&](auto sv) {
                    opts.ejudge.loginPassword.password = util::str(sv);
                });

        parser
            .add("login")
            .required()
            .description("Ejudge login")
            .default_value("config{ejudge.login}")
            .handle([&](auto sv) {
                opts.ejudge.loginPassword.login = util::str(sv);
            });

        parser
            .add("contest_id")
            .required()
            .description("Ejudge contest_id")
            .default_value("config{ejudge.contest_id}")
            .handle<i64>([&](auto id) {
                opts.ejudge.contestId = id;
            });

        parser
            .add("url")
            .optional()
            .default_value("https://caos.ejudge.ru/ej/client?contest_id=@{contest_id}")
            .description("Ejudge page url")
            .store(opts.ejudge.url);
    }

    parser
        .add_help('h', "help");

    return parser;
}

}
#endif // qweqweqwe
