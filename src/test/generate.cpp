#include "cacos/test/generate.h"

#include "cacos/options.h"
#include "cacos/common_args.h"
#include "cacos/util/split.h"

#include <cpparg/cpparg.h>

namespace cacos::test {

template<typename T>
struct Range {
    std::string var;
    T from;
    T to;
    T step;
};

using iRange = Range<int>;

int generate(int argc, const char* argv[]) {
    cpparg::parser parser("cacos test gen");
    parser.title("Generate new tests");

    Options opts;
    setCommonOptions(parser, opts);

    std::string generator;
    parser
        .add('g', "generator")
        .optional()
        .value_type("SOURCE")
        .default_value("@config:generator")
        .description("Generator executable or source")
        .store(generator);
    std::vector<iRange> ranges;
    parser
        .add('r', "range")
        .optional()
        .repeatable()
        .value_type("VAR:FROM:TO:STEP")
        .description("Range")
        .handle([](std::string_view s) {
            std::vector<std::string_view> splitted = util::split(s, ":");
        });
    parser.parse(argc, argv);
    return 0;
}

} // namespace cacos::test
