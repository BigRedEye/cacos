#include "cacos/ejudge/status.h"
#include "cacos/ejudge/http/client.h"
#include "cacos/ejudge/html/myhtml.h"

#include "cacos/util/split.h"
#include "cacos/util/ranges.h"

#include "cacos/common_args.h"
#include "cacos/config.h"
#include "cacos/options.h"

#include <cpparg/cpparg.h>

#include <fmt/format.h>
#include <termcolor/termcolor.hpp>

#include <cpptoml.h>

namespace cacos::ejudge::commands {

int status(int argc, const char* argv[]) {
    cpparg::parser parser("cacos ejudge status");
    parser.title("Get ejudge contest status");

    config::Config cfg(parser, config::EJUDGE);

    parser.parse(argc, argv);

    http::Client client("cookies.txt");

    InlineVariables vars;
    vars.set("contest_id", util::to_string(cfg.ejudge().contestId));

    std::string loginPage = client.post(
        vars.parse(cfg.ejudge().url),
        util::join("login=", cfg.ejudge().login.login.value(), "&password=", cfg.ejudge().login.password.value())
    );

    html::Html page(loginPage);
    html::Collection tags = page.attrs("href");
    std::optional<std::string> url;
    for (auto node : tags) {
        url = node.attr("href").value();
    }

    if (!url) {
        throw std::runtime_error("Cannot parse ejudge responce: href not found");
    }

    // https://caos.ejudge.ru/ej/client/main-page/Sa1a9fcc0f19fe4a1?lt=1
    //                                            ^^^^^^^^^^^^^^^^^
    auto tokens = util::split(*url, "/?");
    if (tokens.size() < 2) {
        throw std::runtime_error("Cannot parse ejudge responce: weird link");
    }
    std::string_view token = tokens[tokens.size() - 2];
    // std::string_view token = "S87b69b1354f650d3";
    // TODO: check for " []: Error: Invalid session"
    if (token.size() != 17) { // Triggered
        throw std::runtime_error("Cannot parse ejudge responce: weird token");
    }

    std::string baseUrl = vars.parse(cfg.ejudge().url);
    std::string_view prefix = util::split(baseUrl, "?")[0];

    std::string res = client.get(util::join(prefix, "/view-problem-summary/", token));
    html::Html summary(res);

    for (auto table : summary.attrs("class", "table")) {
        for (auto row : util::skip(*util::select(table, 1).begin(), 1)) {
            if (row.tag() != MyHTML_TAG_TR) {
                continue;
            }

            std::vector<html::Node> children;
            for (auto child : row) {
                children.push_back(child);
            }
            if (children.size() != 13) {
                throw std::runtime_error("Cannot parse ejudge responce: weird summary table size " + util::to_string(children.size()));
            }
            std::string_view task = children[1].child()->text();
            std::string_view status = children[5].child()->text();
            std::string_view score = /*util::from_string<i32>(*/children[9].child()->text();/*);*/
    
            static constexpr std::string_view nbsp = "\xc2\xa0";

            std::cout << termcolor::reset;
            if (status == "OK" || status == "OK (auto)") {
                std::cout << termcolor::green << termcolor::bold;
            } else if (status == "Rejected") {
                std::cout << termcolor::yellow << termcolor::bold;
            } else if (status == "Partial solution") {
                std::cout << termcolor::red;
            } else if (status == "Pending review") {
                std::cout << termcolor::green;
            }
            if (status != nbsp) {
                if (score != nbsp) {
                    std::cout << task << ": " << status << ", score = " << score << std::endl;
                } else {
                    std::cout << task << ": " << status << std::endl;
                }
            } else {
                std::cout << task << ": --" << std::endl;
            }
        }
    }

    for (auto node : summary.tags(MyHTML_TAG__TEXT)) {
        if (util::starts_with(node.text(), "Total score:")) {
            std::cout << termcolor::reset << termcolor::bold << "Total score: " << node.text().substr(std::string_view("Total score: ").size()) << std::endl;
        }
    }

    return 0;
}

}
