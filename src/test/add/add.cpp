#include "cacos/test/add/add.h"

#include "cacos/config/config.h"
#include "cacos/test/suite/suite.h"

#include <cpparg/cpparg.h>

namespace cacos::test {

template<test::Type type>
int add_impl(int argc, const char* argv[]) {
    std::string descr;
    if constexpr (type == Type::diff) {
        descr = "diff";
    } else if constexpr (type == Type::canonical) {
        descr = "canonical";
    } else {
        throw std::logic_error("Invalid type");
    }

    cpparg::parser parser("cacos test add " + descr);
    parser.title("Add new " + descr + " test");

    Test test;

    test.type(type);

    // clang-format off
    parser
        .add("name")
        .required()
        .value_type("STRING")
        .description("Test name")
        .handle([&](auto sv) {
            test.name(util::str(sv));
        });

    parser
        .add("input")
        .required()
        .value_type("FILE")
        .description("Test input")
        .handle([&](auto sv) {
            test.input(util::str(sv));
        });

    if constexpr (type == Type::canonical) {
        parser
            .add("output")
            .required()
            .value_type("FILE")
            .description("Test output")
            .handle([&](auto sv) {
                test.output(util::str(sv));
            });
    }

    bp::environment env;
    parser
        .add("env")
        .optional()
        .repeatable()
        .value_type("KEY=VALUE")
        .description("Test environment")
        .handle([&](std::string_view sv) {
            auto splitted = util::split(sv, "=");
            if (splitted.size() != 2) {
                throw std::runtime_error("Environment variable should be in form KEY=VALUE");
            }
            env.emplace(util::str(splitted[0]), util::str(splitted[1]));
        });

    std::vector<std::string> args;
    parser
        .add("arg")
        .optional()
        .repeatable()
        .value_type("ARG")
        .description("Test arguments")
        .append(args);

    bool force = false;
    parser
        .add("force")
        .optional()
        .no_argument()
        .description("Overwrite existsing tests")
        .handle([&](auto) {force = true;});
    // clang-format on

    config::Config cfg(parser);
    cfg.ensureWorkspaceExistence();

    parser.parse(argc, argv);

    test.env(env);

    test.serialize(cfg.dir(config::DirType::test), force);

    return 0;
}

int add(int argc, const char* argv[]) {
    cpparg::command_parser parser("cacos test add");
    parser.title("Add new test");

    // clang-format off
    parser
        .command("canonical")
        .description("Add new canonical test")
        .handle(cacos::test::add_impl<Type::canonical>);

    parser
        .command("diff")
        .description("Add new diff test")
        .handle(cacos::test::add_impl<Type::diff>);
    // clang-format on

    return parser.parse(argc, argv);
}

} // namespace cacos::test
