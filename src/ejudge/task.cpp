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

    std::cout << client.source(id);

    return 0;
}

int dump(int argc, const char* argv[]) {
    cpparg::parser parser("cacos task run dump");
    parser.title("Download all solutions");

    config::Config cfg(parser, config::EJUDGE_SESSION);

    fs::path output;
    // clang-format off
    parser
        .add('o', "output")
        .required()
        .description("Path to output directory")
        .value_type("DIR")
        .store(output);
    // clang-format on

    parser.parse(argc, argv);

    if (fs::exists(output)) {
        if (!fs::is_directory(output)) {
            throw std::runtime_error{"Output path already exists"};
        }
        if (!fs::is_empty(output)) {
            throw std::runtime_error{"Output path is not empty"};
        }
    } else {
        fs::create_directories(output);
    }

    parser::Parser client(cfg);

    auto tasks = client.tasks();
    for (auto&& task : tasks) {
        fs::create_directory(output / task.name);
        auto submits = client.solutions(task.id);
        for (auto&& submit : submits) {
            std::ofstream ofs{output / task.name / util::string::join(submit.result.status, '_', submit.id)};
            ofs << client.source(submit.id);
        }
    }

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
    cpparg::command_parser parser("cacos task run");
    parser.title("Manage runs");

    // clang-format off
    parser
        .command("list")
        .description("List runs")
        .handle(list);

    parser
        .command("get")
        .description("Download run code")
        .handle(fetch);

    parser
        .command("dump")
        .description("Download all solutions")
        .handle(dump);
    // clang-format on

    return parser.parse(argc, argv);
}

int task(int argc, const char* argv[]) {
    cpparg::command_parser parser("cacos task");
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
