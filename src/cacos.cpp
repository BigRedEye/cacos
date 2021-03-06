#include "cacos/commands/config.h"
#include "cacos/commands/init.h"

#include "cacos/test/test.h"

#include "cacos/ejudge/status.h"
#include "cacos/ejudge/task.h"

#include "cacos/version/version.h"

#include <cpparg/cpparg.h>

#include <iostream>

namespace cacos {

int main(int argc, const char* argv[]) {
    // IO hangs by default at tty (sync inside asio callback fails)
    std::ios_base::sync_with_stdio(false);

    cpparg::command_parser parser("cacos");
    parser.title("CAOS testing utility");

    // clang-format off
    parser
        .command("init")
        .description("Initialize new workspace")
        .handle(cacos::commands::init);

    parser
        .command("config")
        .description("Manage config")
        .handle(cacos::commands::config);

    parser
        .command("test")
        .description("Manage tests")
        .handle(cacos::commands::test);

    parser
        .command("status")
        .description("Ejudge contest status")
        .handle(cacos::ejudge::commands::status);

    parser
        .command("task")
        .description("Manage ejudge tasks")
        .handle(cacos::ejudge::commands::task);

    parser
        .command("version")
        .description("Print cacos version")
        .handle(cacos::commands::version);
    // clang-format on

    return parser.parse(argc, argv);
}

} // namespace cacos
