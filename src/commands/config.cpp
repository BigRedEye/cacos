#include "cacos/commands/config.h"
#include "cacos/config/config.h"

#include <cpparg/cpparg.h>

namespace cacos::commands {

int config(int argc, const char* argv[]) {
    cpparg::parser parser("cacos config");
    parser.title("Manage config");

    config::Config cfg;

    parser
        .add("login")
        .optional()
        .description("Ejudge login")
        .handle([&](auto sv) {
            cfg.set(sv, "ejudge.login");
        });

    parser
        .add("password")
        .optional()
        .description("Ejudge password")
        .default_value("config{ejudge.password}")
        .handle([&, this](auto sv) {
            cfg.set(sv, "ejudge.login");
        });

    parser
        .add("cookie")
        .optional()
        .description("Ejudge cookie (EJSID=...)")
        .default_value("config{ejudge.cookie}")
        .handle([&](auto sv) {
            ejudge_.session.ejsid = argOrConfig<std::string>(util::str(sv), "ejudge.cookie");
        });

    parser
        .add("token")
        .optional()
        .description("Ejudge session token from url")
        .default_value("config{ejudge.token")
        .handle([&](auto sv) {
            ejudge_.session.token = argOrConfig<std::string>(util::str(sv), "ejudge.token");
        });

    parser
        .add("contest_id")
        .optional()
        .description("Ejudge contest_id")
        .default_value("config{ejudge.contest_id}")
        .handle([&](auto sv) {
            ejudge_.contestId = argOrConfig<i32>(util::str(sv), "ejudge.contest_id", ConfigError("Invalid contest_id"));
        });

    parser
        .add("url")
        .optional()
        .description("Ejudge login page")
        .default_value("https://caos.ejudge.ru/ej/client?config{contest_id}")
        .handle([&](auto url) {
            ejudge_.url = argOrConfig<std::string>(util::str(url), "ejudge.url", ConfigError("Invalid ejudge url"));
        });

    return 0;
}

}
