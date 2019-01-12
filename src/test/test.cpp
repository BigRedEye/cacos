#include "cacos/config.h"
#include "cacos/test/test.h"
#include "cacos/util/util.h"
#include "cacos/common_args.h"

#include "cacos/test/generate/generate.h"
#include "cacos/test/add/add.h"

#include <cpparg/cpparg.h>

/*

cacos test gen --generator gen.c --name test-@i-@j --range i:0:5:1 --range j:0:5:1 -- @i @j

*/

namespace cacos::commands {

int test(int argc, const char* argv[]) {
    cpparg::command_parser parser("cacos test");
    parser.title("Manage tests");

    parser.command("add").description("Add new test").handle(cacos::test::add);
    parser.command("gen").description("Generate new tests").handle(cacos::test::generate);

    return parser.parse(argc, argv);
}

int run(int argc, const char* argv[]) {
    cpparg::parser parser("cacos run");
    parser.title("Compile and run");

    Options opts;
    cacos::setCommonOptions(parser, opts);

    parser.parse(argc, argv);

    return 0;
}

}
