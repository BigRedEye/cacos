#include "cacos/test/generate/generate.h"
#include "cacos/test/generate/generator.h"

#include "cacos/test/suite/test.h"

#include "cacos/config/config.h"
#include "cacos/util/split.h"
#include "cacos/util/string.h"

#include <cpparg/cpparg.h>

#include <map>
#include <string>

namespace cacos::test {

template<Type type>
int generateImpl(int argc, const char* argv[]) {
    cpparg::parser parser("cacos test gen");
    parser.title("Generate new tests");

    GeneratorOptions opts;

    // clang-format off
    parser
        .add("generator")
        .required()
        .value_type("SOURCE")
        .description("Generator executable or sources")
        .handle([&](auto sv) {
            auto splitted = util::split(sv, ", ");
            for (auto&& path : splitted) {
                if (!path.empty()) {
                    opts.generatorSources.emplace_back(path);
                }
            }
        });

    parser
        .add("for")
        .optional()
        .repeatable()
        .value_type("VAR:FROM:TO:STEP")
        .description("Declares variable VAR that can be used via @{VAR}")
        .handle([&](std::string_view s) {
            std::vector<std::string_view> splitted = util::split(s, ":,. ");
            if (splitted.empty()) {
                throw std::runtime_error("Invalid range");
            }
            std::string name = util::str(splitted[0]);
            while (splitted.size() < 4) {
                splitted.push_back("1");
            }

            Range<i64> range {
                util::string::from<i64>(splitted[1]),
                util::string::from<i64>(splitted[2]),
                util::string::from<i64>(splitted[3]),
            };

            opts.vars[name] = range;
        });

    parser
        .add("args")
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
        .add("stdin")
        .optional()
        .value_type("STRING")
        .description("Stdin for generator")
        .store(opts.input);

    parser
        .add("test")
        .required()
        .value_type("STRING")
        .description("Test name")
        .store(opts.testName);
    // clang-format on

    opts.type = type;

    config::Config cfg(parser, config::LANGS);

    parser.parse(argc, argv);
    Generator generator(cfg, opts);
    generator.run();

    return 0;
}

int generate(int argc, const char* argv[]) {
    cpparg::command_parser parser("cacos test generate");
    parser.title("Test generation helper");

    // clang-format off
    parser
        .command("canonical")
        .description("Generate canonical tests")
        .handle(generateImpl<Type::canonical>);

    parser
        .command("diff")
        .description("Generate diff tests")
        .handle(generateImpl<Type::diff>);
    // clang-format on

    return parser.parse(argc, argv);
}

} // namespace cacos::test
