#include "cacos/ejudge/status.h"
#include "cacos/ejudge/http/client.h"
#include "cacos/ejudge/html/myhtml.h"
#include "cacos/ejudge/session.h"

#include "cacos/config/config.h"

#include "cacos/util/split.h"
#include "cacos/util/ranges.h"

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

    http::Client client(cfg.file(config::FileType::cookies));
    Session session(client, cfg);

    html::Html summary = session.get("view-problem-summary");

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
