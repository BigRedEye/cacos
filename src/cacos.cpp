#include "cacos/commands/init.h"
#include "cacos/commands/config.h"
#include "cacos/test/test.h"
#include "cacos/ejudge/status.h"

#include <cpparg/cpparg.h>

#include <iostream>

#include <cpptoml.h>
#include <string_view>

namespace cacos {

int main(int argc, const char* argv[]) {
    // IO hangs by default at tty (sync inside asio callback fails)
    std::ios_base::sync_with_stdio(false);

    cpparg::command_parser parser("cacos");
    parser.title("CAOS testing utility");
    parser.command("init").description("Initialize new workspace").handle(cacos::commands::init);
    parser.command("test").description("Manage tests").handle(cacos::commands::test);
    parser.command("run").description("Compile and run").handle(cacos::commands::run);
    parser.command("status").description("Ejudge contest status").handle(cacos::ejudge::commands::status);
    parser.command("config").description("Manage config").handle(cacos::commands::config);

    return parser.parse(argc, argv);
}

} // namespace cacos
