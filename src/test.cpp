#include "cacos/config.h"
#include "cacos/test.h"
#include "cacos/util.h"
#include "cacos/common_args.h"

#include <cpparg/cpparg.h>

namespace cacos::commands {

int test(int argc, const char* argv[]) {
    return !!argv[argc];
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
