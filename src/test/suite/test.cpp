#include "cacos/test/suite/test.h"

#include <cpptoml.h>

namespace cacos::test {

Test::Test(const fs::path& toml) {
    if (!fs::exists(toml)) {
        throw std::runtime_error("Cannot load test file " + toml.string());
    }

    auto table = cpptoml::parse_file(toml.string());

    if (!table) {
        throw std::runtime_error("Cannot parse test toml for " + toml.string());
    }

    auto test_table = table->get_table("test");

    if (!test_table) {
        throw std::runtime_error("Cannot find table [test] in " + toml.string());
    }

    auto get = [&] (bool required, const std::string& key, auto dummy = {})
            -> std::optional<std::decay_t<decltype(dummy)>> {
        if (auto node = test_table->get_as<std::decay_t<decltype(dummy)>>(key)) {
            return *node;
        } else if (required) {
            throw std::runtime_error("Cannot find test." + key + " in " + toml.string());
        } else {
            return std::nullopt;
        }
    };

    if (auto test = get(true, "key", std::string{})) {
        if (util::string::starts_with(*test, "diff")) {
            type_ = Type::diff;
        } else if (util::string::starts_with(*test, "canon")) {
            type_ = Type::canonical;
        } else {
            throw std::runtime_error("Unknown test type " + *test);
        }
    }

    if (auto input = get(true, "stdin", std::string{})) {
        input_ = *input;
    }

    if (type_ == Type::canonical) {
        if (auto output = get(true, "stdout", std::string{})) {
            output_ = *output;
        }
    }

    if (auto env = test_table->get_table("env")) {
        for (auto [k, t] : *env) {
            if (auto v = t->as<std::string>()) {
                env_.emplace(k, v->get());
            }
        }
    }

    if (auto args = test_table->get_array_of<std::string>("args")) {
        args_ = std::move(*args);
    }
}

executable::ExecTaskPtr Test::task(TaskContext&& context) {
    executable::ExecTaskContext ctx {
        args_,
        env_,
        std::move(context.callback),
    };

    if (context.stdErr) {
        return executable::makeTask(context.exe, std::move(ctx), input_, context.stdOut, *context.stdErr);
    } else {
        return executable::makeTask(context.exe, std::move(ctx), input_, context.stdOut, bp::null);
    }
}

TestingResult Test::compare(const fs::path& output_file) {
    if (type_ != Type::canonical) {
        throw std::logic_error("Invalid test type");
    }
    return compare(output_file, output_.value());
}

TestingResult Test::compare(const fs::path& outputFile, const fs::path& expectedOutput) {
    /// TODO: src/util/diff.cpp
    /// Calculate unidiff chunks
    /// Use UniDiff instead of std::string in TestingResult
    /// Add print(UniDiff, std::ostream&)
}

}
