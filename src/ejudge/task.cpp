#include "cacos/ejudge/task.h"
#include "cacos/ejudge/diff.h"
#include "cacos/ejudge/parser/parser.h"
#include "cacos/ejudge/session.h"

#include "cacos/config/config.h"

#include "cacos/ejudge/html/printer.h"

#include <cpparg/cpparg.h>

namespace cacos::ejudge::commands {

int list(int argc, const char* argv[]) {
    cpparg::parser parser("cacos task run list");
    parser.title("List solutions");

    config::Config cfg(parser, config::EJUDGE_SESSION);

    std::string taskName;
    // clang-format off
    parser
        .positional("task")
        .required()
        .description("Ejudge task (sm02-3, for example)")
        .value_type("TASK")
        .store(taskName);
    // clang-format on

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
    cpparg::parser parser("cacos task run get");
    parser.title("Get solution code");

    config::Config cfg(parser, config::EJUDGE_SESSION);

    i32 id;
    // clang-format off
    parser
        .positional("id")
        .required()
        .description("Ejudge run id")
        .value_type("ID")
        .store(id);
    // clang-format on

    parser.parse(argc, argv);

    parser::Parser client(cfg);

    std::cout << client.source(id) << std::endl;

    return 0;
}

int statement(int argc, const char* argv[]) {
    cpparg::parser parser("cacos task statement");
    parser.title("Print task statement");

    config::Config cfg(parser, config::EJUDGE_SESSION);

    std::string taskName;
    // clang-format off
    parser
        .positional("task")
        .required()
        .description("Ejudge task (sm02-3, for example)")
        .value_type("TASK")
        .store(taskName);
    // clang-format on

    parser.parse(argc, argv);

    parser::Parser client(cfg);

    if (auto task = client.task(taskName)) {
        auto [page, statement] = client.statement(task->id);
        for (auto node = statement.begin(); node != statement.end(); node = node.next()) {
            html::print(node);
        }
    } else {
        throw std::runtime_error("Unknodwn task '" + taskName + "'");
    }

    return 0;
}

int run(int argc, const char* argv[]) {
    cpparg::command_parser parser("cacos ejudge task");
    parser.title("Manage ejudge tasks");

    // clang-format off
    parser
        .command("list")
        .description("List runs")
        .handle(list);

    parser
        .command("get")
        .description("Download run code")
        .handle(fetch);
    // clang-format on

    return parser.parse(argc, argv);
}

int task(int argc, const char* argv[]) {
    cpparg::command_parser parser("cacos ejudge task");
    parser.title("Manage ejudge tasks");

    // clang-format off
    parser
        .command("run")
        .description("Manage runs")
        .handle(run);

    parser
        .command("diff")
        .description("Compare solutions")
        .handle(diff);

    parser
        .command("statement")
        .description("View task statement")
        .handle(statement);
    // clang-format on

    return parser.parse(argc, argv);
}

} // namespace cacos::ejudge::commands
