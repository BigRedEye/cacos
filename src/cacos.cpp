#include "cacos/init.h"
#include "cacos/config.h"
#include "cacos/test/test.h"

#include <cpparg/cpparg.h>

#include <iostream>

#include <cpptoml.h>
#include <string_view>

namespace cacos {

int main(int argc, const char* argv[]) {
    cpparg::command_parser parser("cacos");
    parser.title("CAOS testing utility");
    parser.command("init").description("Initialize new workspace").handle(cacos::commands::init);
    parser.command("test").description("Manage tests").handle(cacos::commands::test);
    parser.command("run").description("Compile and run").handle(cacos::commands::run);
    return parser.parse(argc, argv);
}

} // namespace cacos
