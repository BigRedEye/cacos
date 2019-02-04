#include "cacos/ejudge/status.h"
#include "cacos/ejudge/html/myhtml.h"
#include "cacos/ejudge/http/client.h"
#include "cacos/ejudge/parser/parser.h"
#include "cacos/ejudge/session.h"

#include "cacos/config/config.h"

#include "cacos/util/ranges.h"
#include "cacos/util/split.h"

#include <cpparg/cpparg.h>

#include <fmt/format.h>
#include <termcolor/termcolor.hpp>

namespace cacos::ejudge::commands {

int status(int argc, const char* argv[]) {
    cpparg::parser parser("cacos ejudge status");
    parser.title("Get ejudge contest status");

    config::Config cfg(parser, config::EJUDGE);

    parser.parse(argc, argv);

    parser::Parser client(cfg);

    for (auto&& task : client.tasks()) {
        std::cout << termcolor::reset;
        if (task.result) {
            const std::string& status = task.result->status;
            if (status == "OK" || status == "OK (auto)") {
                std::cout << termcolor::green << termcolor::bold;
            } else if (status == "Rejected") {
                std::cout << termcolor::yellow << termcolor::bold;
            } else if (status == "Partial solution") {
                std::cout << termcolor::red;
            } else if (status == "Pending review") {
                std::cout << termcolor::green;
            }
            std::cout << task.name << ": " << status;
            if (task.result->score) {
                std::cout << ", score = " << *task.result->score;
            }
        } else {
            std::cout << task.name << ": --";
        }
        std::cout << std::endl;
    }

    std::cout << termcolor::bold << "Total score: " << client.score() << termcolor::reset
              << std::endl;

    return 0;
}

} // namespace cacos::ejudge::commands
