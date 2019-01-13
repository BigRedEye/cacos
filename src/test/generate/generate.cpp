#include "cacos/test/generate/generate.h"
#include "cacos/test/generate/generator.h"

#include "cacos/options.h"
#include "cacos/common_args.h"
#include "cacos/util/split.h"
#include "cacos/util/string.h"

#include <cpparg/cpparg.h>

#include <string>
#include <map>

namespace cacos::test {

int generate(int argc, const char* argv[]) {
    cpparg::parser parser("cacos test gen");
    parser.title("Generate new tests");

    GeneratorOptions opts;
    setCommonOptions(parser, opts);

    parser
        .add('g', "generator")
        .required()
        .value_type("SOURCE")
        .description("Generator executable or source")
        .store(opts.generator);
    parser
        .add('v', "var")
        .optional()
        .repeatable()
        .value_type("VAR:FROM:TO:STEP")
        .description("Variable")
        .handle([&](std::string_view s) {
            std::vector<std::string_view> splitted = util::split(s, ":");
            if (splitted.empty()) {
                throw std::runtime_error("Invalid range");
            }
            std::string name = util::str(splitted[0]);
            while (splitted.size() < 4) {
                splitted.push_back("1");
            }

            Range<int> range {
                util::from_string<int>(splitted[1]),
                util::from_string<int>(splitted[2]),
                util::from_string<int>(splitted[3]),
            };

            opts.vars[name] = range;
        });

    parser
        .add('a', "args")
        .optional()
        .value_type("ARGS")
        .description("Arguments for generator, separated with spaces")
        .handle([&] (std::string_view s) {
            std::vector<std::string_view> splitted = util::split(s, " ");
            for (auto&& sv : splitted) {
                opts.args.push_back(util::str(sv));
            }
        });

    parser
        .add('i', "stdin")
        .optional()
        .value_type("STRING")
        .description("Stdin for generator")
        .store(opts.input);

    parser
        .add('t', "test")
        .optional()
        .value_type("STRING")
        .description("Test name")
        .store(opts.testName);

    parser.parse(argc, argv);

    Generator generator(opts);
    generator.run();

    return 0;
}

} // namespace cacos::test