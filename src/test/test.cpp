#include "cacos/test/test.h"
#include "cacos/config/config.h"
#include "cacos/test/run/run.h"
#include "cacos/util/util.h"

#include "cacos/test/add/add.h"
#include "cacos/test/generate/generate.h"

#include <cpparg/cpparg.h>

namespace cacos::commands {

int test(int argc, const char* argv[]) {
    cpparg::command_parser parser("cacos test");
    parser.title("Manage tests");

    parser.command("add").description("Add new test").handle(cacos::test::add);
    parser.command("gen").description("Generate new tests").handle(cacos::test::generate);
    parser.command("run").description("Run tests").handle(cacos::test::run);

    return parser.parse(argc, argv);
}

} // namespace cacos::commands
