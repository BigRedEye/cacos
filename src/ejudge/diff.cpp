#include "cacos/ejudge/diff.h"
#include "cacos/ejudge/parser/parser.h"

#include "cacos/config/config.h"

#include "cacos/util/diff/unified.h"

#include <cpparg/cpparg.h>

namespace cacos::ejudge::commands {

int diff(int argc, const char* argv[]) {
    cpparg::parser parser("cacos task diff");
    parser.title("Compare solutions and local files");

    config::Config cfg(parser, config::EJUDGE_SESSION);

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

} // namespace cacos::ejudge::commands
