#include "cacos/ejudge/parser/parser.h"
#include "cacos/ejudge/session.h"
#include "cacos/ejudge/solution.h"

#include "cacos/config/config.h"

#include <cpparg/cpparg.h>

namespace cacos::ejudge::commands {

int list(int argc, const char* argv[]) {
    cpparg::parser parser("cacos ejudge solution list");
    parser.title("List solutions");

    config::Config cfg(parser, config::EJUDGE);

    std::string taskName;
    parser
        .positional("task")
        .required()
        .description("Ejudge task (sm02-3, for example)")
        .value_type("TASK")
        .store(taskName);

    parser.parse(argc, argv);

    parser::Parser client(cfg);

    if (auto task = client.task(taskName)) {
        std::cout << "Task id: " << task->id << std::endl;
        auto solutions = client.solutions(task->id);
        for (auto&& solution : solutions) {
            std::cout << solution.id << "\t" << solution.result.status << std::endl;
        }
    } else {
        throw std::runtime_error("Cannot find task with name \"" + taskName + "\"");
    }

    return 0;
}

int fetch(int argc, const char* argv[]) {
    cpparg::parser parser("cacos ejudge solution fetch");
    parser.title("Get solution code");

    config::Config cfg(parser, config::EJUDGE);

    i32 id;
    parser
        .positional("id")
        .required()
        .description("Ejudge run id")
        .value_type("ID")
        .store(id);

    parser.parse(argc, argv);

    parser::Parser client(cfg);

    std::cout << client.source(id) << std::endl;

    /*
    if (auto task = client.source(taskName)) {
    } else {
        throw std::runtime_error("Cannot find task with name \"" + taskName + "\"");
    }
    */

    return 0;
}

int solution(int argc, const char* argv[]) {
    cpparg::command_parser parser("cacos ejudge solution");
    parser.title("Manage ejudge solutions");

    parser.command("list").description("List solutions").handle(list);
    parser.command("fetch").description("Fetch solution").handle(fetch);

    return parser.parse(argc, argv);
}

}
