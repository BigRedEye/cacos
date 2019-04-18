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
        .add("name")
        .required()
        .value_type("STRING")
        .description("Test name")
        .store(opts.name);

    parser
        .add("generator")
        .required()
        .value_type("SOURCE")
        .description("Generator executable or sources")
        .handle([&](auto sv) {
            auto splitted = util::split(sv, ", ");
            for (auto&& path : splitted) {
                if (path.empty()) {
                    continue;
                }
                fs::path source = path;
                if (source.is_relative()) {
                    source = fs::absolute(source);
                }
                if (!fs::exists(source)) {
                    throw std::runtime_error(util::string::join("File ", source.string(), " does not exist"));
                }
                opts.generatorSources.emplace_back(source);
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
        .add("gen.arg")
        .optional()
        .value_type("ARG")
        .description("Argument for generator")
        .repeatable()
        .append(opts.args);

    parser
        .add("gen.env")
        .optional()
        .value_type("ENV")
        .description("Environment variable for generator in form KEY=VALUE")
        .repeatable()
        .handle([&](std::string_view sv) {
            size_t pos = sv.find('=');
            if (pos == std::string_view::npos) {
                throw std::runtime_error(util::string::join("Invalid environment variable ", sv));
            }
            std::string_view key = sv.substr(0, pos);
            std::string_view value = sv.substr(pos + 1);
            opts.env.set(util::str(key), util::str(value));
        });

    parser
        .add("gen.stdin")
        .optional()
        .default_value(" ")
        .value_type("STRING")
        .description("Stdin for generator")
        .store(opts.genIO.input);

    parser
        .add("test.stdin")
        .optional()
        .value_type("FILE")
        .description("Test stdin")
        .default_value("gen.stdout")
        .store(opts.testIO.input);

    parser
        .add("threads")
        .optional()
        .description("Process limit")
        .default_value(util::string::to(std::thread::hardware_concurrency()))
        .handle<size_t>([&](size_t threads) { opts.threads = threads; });

    if constexpr (type == Type::canonical) {
        parser
            .add("test.stdout")
            .optional()
            .value_type("FILE")
            .description("Canonical test stdout")
            .default_value("gen.stderr")
            .store(opts.testIO.output);
    }

    parser
        .add("test.arg")
        .optional()
        .value_type("ARG")
        .description("Argument for test")
        .repeatable()
        .append(opts.testArgs);

    parser
        .add("force")
        .optional()
        .no_argument()
        .description("Overwrite existsing tests")
        .handle([&](auto) {
            opts.force = true;
        });
    // clang-format on

    opts.type = type;

    config::Config cfg(parser, config::LANGS | config::TASK_EXE | config::KEEP_WORKING_DIRS);

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
