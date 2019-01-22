#include "cacos/ejudge/status.h"
#include "cacos/ejudge/http/client.h"

#include "cacos/util/string.h"

#include "cacos/common_args.h"
#include "cacos/config.h"
#include "cacos/options.h"

#include <cpparg/cpparg.h>

namespace cacos::ejudge::commands {

int status(int argc, const char* argv[]) {
    cpparg::parser parser("cacos ejudge status");
    parser.title("Get ejudge contest status");

    Options opts;
    setCommonOptions(parser, opts, options::EJUDGE | options::CONFIG);

    parser.parse(argc, argv);

    config::Config cfg(opts);

    http::Client client("cookies.txt");

    InlineVariables vars;
    vars.set("contest_id", util::to_string(opts.ejudge.contestId));

    /*std::cout << client.post(
        vars.parse(opts.ejudge.url),
        util::join("login=", *opts.ejudge.loginPassword.login, "&password=", *opts.ejudge.loginPassword.password)
    ) << std::endl;*/
    std::string url;
    std::cin >> url;

    std::cout << client.get(url) << std::endl;

    return 0;
}

}
