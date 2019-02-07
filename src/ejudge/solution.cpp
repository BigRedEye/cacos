#include "cacos/ejudge/solution.h"
#include "cacos/ejudge/parser/parser.h"
#include "cacos/ejudge/session.h"

#include "cacos/config/config.h"

#include "cacos/util/diff/unified.h"

#include <cpparg/cpparg.h>

namespace cacos::ejudge::commands {

int list(int argc, const char* argv[]) {
    cpparg::parser parser("cacos ejudge solution list");
    parser.title("List solutions");

    config::Config cfg(parser, config::EJUDGE);

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
    cpparg::parser parser("cacos ejudge solution fetch");
    parser.title("Get solution code");

    config::Config cfg(parser, config::EJUDGE);

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

int diff(int argc, const char* argv[]) {
    cpparg::parser parser("cacos ejudge solution diff");
    parser.title("Compare solutions");

    config::Config cfg(parser, config::EJUDGE);

    auto add = [&](auto name) -> auto& {
        // clang-format off
        return parser
            .positional(name)
            .required()
            .description("Ejudge run id or source file")
            .value_type("SOURCE");
        // clang-format on
    };

    std::string ids[2];
    add("first").store(ids[0]);
    add("second").store(ids[1]);

    parser.parse(argc, argv);

    parser::Parser client(cfg);

    std::string sources[2];

    auto getSource = [&](std::string_view id) {
        std::optional<i32> runId;
        try {
            runId = util::string::from<i32>(id);
        } catch (...) {
            // ¯\_(ツ)_/¯
        }
        if (runId) {
            return util::str(client.source(runId.value()));
        } else if (fs::exists(id)) {
            return util::file::read(id);
        } else {
            throw std::runtime_error("Cannot find file " + util::str(id));
        }
    };

    util::diff::Unified uni(getSource(ids[0]), getSource(ids[1]));
    uni.print(std::cout);

    return 0;
}

int solution(int argc, const char* argv[]) {
    cpparg::command_parser parser("cacos ejudge solution");
    parser.title("Manage ejudge solutions");

    // clang-format off
    parser
        .command("list")
        .description("List solutions")
        .handle(list);

    parser
        .command("fetch")
        .description("Fetch solution")
        .handle(fetch);

    parser
        .command("diff")
        .description("Compare solutions")
        .handle(diff);
    // clang-format on

    return parser.parse(argc, argv);
}

} // namespace cacos::ejudge::commands
