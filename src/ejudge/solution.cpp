#include "cacos/ejudge/solution.h"
#include "cacos/ejudge/parser/parser.h"
#include "cacos/ejudge/session.h"

#include "cacos/config/config.h"

#include <cpparg/cpparg.h>

#include <dtl/dtl.hpp>

#include <termcolor/termcolor.hpp>

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

    std::string rawSources[2];

    auto splitSource = [&] (size_t id) {
        std::optional<i32> runId;
        try {
            runId = util::string::from<i32>(ids[id]);
        } catch (...) {
            // ¯\_(ツ)_/¯
        }
        if (runId) {
            rawSources[id] = client.source(runId.value());
        } else if (fs::exists(ids[id])) {
            rawSources[id] = util::file::read(ids[id]);
        } else {
            throw std::runtime_error("Cannot find file " + ids[id]);
        }
        return util::split(rawSources[id], "\n");
    };

    std::vector<std::string_view> sources[] = {splitSource(0),
                                               splitSource(1)};

    dtl::Diff<std::string_view> d(sources[0], sources[1]);
    d.compose();
    d.composeUnifiedHunks();
    for (auto&& hunk : d.getUniHunks()) {
        std::cout << termcolor::bold << termcolor::cyan;
        fmt::print(std::cout, "@@ -{},{} +{},{} @@\n", hunk.a, hunk.b, hunk.c, hunk.d);
        std::cout << termcolor::reset;
        for (auto&& line : hunk.common[0]) {
            fmt::print(std::cout, " {}\n", line.first);
        }
        for (auto&& line : hunk.change) {
            switch (line.second.type) {
            case dtl::SES_ADD:
                std::cout << termcolor::green << "+";
                break;
            case dtl::SES_COMMON:
                std::cout << termcolor::reset << " ";
                break;
            case dtl::SES_DELETE:
                std::cout << termcolor::red << "-";
                break;
            }
            fmt::print(std::cout, "{}\n", line.first);
        }
        std::cout << termcolor::reset;
        for (auto&& line : hunk.common[1]) {
            fmt::print(std::cout, " {}\n", line.first);
        }
    }

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
