#include "cacos/test/suite/test.h"

#include "cacos/util/diff/unified.h"

#include "cacos/util/logger.h"
#include "cacos/util/split.h"
#include "cacos/util/string.h"

#include <cpptoml.h>

#include <fstream>

namespace cacos::test {

std::string serialize(Type type) {
    switch (type) {
    case Type::diff:
        return "diff";
    case Type::canonical:
        return "canonical";
    default:
        return "unknown";
    }
}

BlobDiff::BlobDiff(const std::string& left, const std::string& right)
    : left_(util::string::escape(left))
    , right_(util::string::escape(right)) {
}

const std::string& BlobDiff::left() const {
    return left_;
}

const std::string& BlobDiff::right() const {
    return right_;
}

Test::Test(const fs::path& base, const fs::path& toml) {
    if (!fs::exists(toml)) {
        throw std::runtime_error("Cannot load test file " + toml.string());
    }

    workingDirectory_ = toml.parent_path();
    name_ = fs::relative(toml.parent_path(), base).string();

    auto table = cpptoml::parse_file(toml.string());

    if (!table) {
        throw std::runtime_error("Cannot parse test toml for " + toml.string());
    }

    auto get = [&](bool required,
                   const std::string& key,
                   auto dummy = {}) -> std::optional<std::decay_t<decltype(dummy)>> {
        if (auto node = table->get_as<std::decay_t<decltype(dummy)>>(key)) {
            return *node;
        } else if (required) {
            throw std::runtime_error("Cannot find " + key + " in " + toml.string());
        } else {
            return std::nullopt;
        }
    };

    if (auto test = get(true, "type", std::string{})) {
        if (util::string::starts(*test, "diff")) {
            type_ = Type::diff;
        } else if (util::string::starts(*test, "canon")) {
            type_ = Type::canonical;
        } else {
            throw std::runtime_error("Unknown test type " + *test);
        }
    }

    if (auto input = get(true, "stdin", std::string{})) {
        input_ = toml.parent_path() / *input;
    }

    if (type_ == Type::canonical) {
        if (auto output = get(true, "stdout", std::string{})) {
            output_ = toml.parent_path() / *output;
        }
    }

    if (auto env = table->get_table("env")) {
        env_ = boost::this_process::environment();
        for (auto [k, t] : *env) {
            if (auto v = t->as<std::string>()) {
                env_->set(k, v->get());
            }
        }
    }

    if (auto args = table->get_array_of<std::string>("args")) {
        args_ = std::move(*args);
    }
}

void Test::serialize(const fs::path& workspace, bool force) {
    fs::path dir = root(workspace);

    if (fs::exists(dir) && !fs::is_empty(dir)) {
        if (force) {
            fs::remove_all(dir);
        } else {
            throw std::runtime_error(
                "Cannot save test: directory " + dir.string() +
                " is not empty. Use '--force' to overwrite");
        }
    }

    fs::create_directories(dir);

    std::ofstream ofs(dir / CONFIG_FILE);

    if (workingDirectory_) {
        for (auto ent : fs::directory_iterator(*workingDirectory_)) {
            fs::path oldp = ent.path();
            if (fs::equivalent(oldp, input_) ||
                (output_ && fs::equivalent(oldp, output_.value()))) {
                continue;
            }
            fs::path newp = dir / fs::relative(oldp, *workingDirectory_);
            fs::rename(oldp, newp);
        }
    }
    workingDirectory_ = dir;

    auto copyIO = [&](fs::path& old, const fs::path& path) {
        fs::copy(old, path);
        old = fs::relative(path, dir);
    };

    copyIO(input_, dir / INPUT_FILE);
    if (output_) {
        copyIO(*output_, dir / OUTPUT_FILE);
    }

    auto table = cpptoml::make_table();

    auto insert = [&](const std::string& key, auto value) {
        table->insert(key, cpptoml::make_value(value));
    };

    insert("type", ::cacos::test::serialize(type_));
    insert("stdin", input_.string());
    if (output_) {
        insert("stdout", output_->string());
    }

    auto args = cpptoml::make_array();
    for (auto&& arg : args_) {
        args->push_back(arg);
    }

    table->insert("args", args);

    if (env_) {
        auto envTable = cpptoml::make_table();
        for (auto&& it : env_.value()) {
            envTable->insert(it.get_name(), it.to_string());
        }
        table->insert("env", envTable);
    }

    ofs << *table;
}

Test& Test::type(Type type) {
    type_ = type;
    return *this;
}

Test& Test::name(const std::string& name) {
    name_ = name;
    return *this;
}

Test& Test::input(const fs::path& input) {
    input_ = input;
    return *this;
}

Test& Test::output(const fs::path& output) {
    if (type_ != Type::canonical) {
        throw std::runtime_error("Cannot add canonical output for diff test");
    }
    output_ = output;
    return *this;
}

Test& Test::workingDirectory(const fs::path& dir) {
    workingDirectory_ = dir;
    return *this;
}

Test& Test::env(const bp::environment& env) {
    env_ = env;
    return *this;
}

Test& Test::args(const std::vector<std::string>& args) {
    args_ = args;
    return *this;
}

Type Test::type() const {
    return type_;
}

const std::string& Test::name() const {
    return name_;
}

int Test::returnCode() const {
    /// TODO: fixme
    return 0;
}

executable::ExecTaskPtr Test::task(TaskContext&& context) const {
    executable::ExecTaskContext ctx{
        args_,
        env_.value_or(boost::this_process::environment()),
        workingDirectory_.value_or(fs::current_path()),
        std::move(context.callback),
    };

    if (context.stdErr) {
        return executable::makeTask(
            context.exe, std::move(ctx), input_, context.stdOut, *context.stdErr);
    } else {
        return executable::makeTask(context.exe, std::move(ctx), input_, context.stdOut, bp::null);
    }
}

namespace {

TestingResult compareInMemory(const fs::path& outputFile, const fs::path& expectedOutput);
TestingResult compareBlobs(const fs::path& outputFile, const fs::path& expectedOutput);

} // namespace

TestingResult Test::compare(
    const fs::path& outputFile,
    const std::optional<fs::path>& expectedOptional) const {
    fs::path expectedOutput;
    if (expectedOptional) {
        expectedOutput = expectedOptional.value();
    } else {
        try {
            expectedOutput = output_.value();
        } catch (const std::bad_optional_access&) {
            std::throw_with_nested(std::logic_error{"Invalid test type"});
        }
    }

    static constexpr bytes diffThreshold = 1 << 8;
    if (fs::file_size(outputFile) < diffThreshold &&
        fs::file_size(expectedOutput) < diffThreshold) {
        return compareInMemory(expectedOutput, outputFile);
    } else {
        return compareBlobs(expectedOutput, outputFile);
    }
}

executable::ExecTaskPtr Test::compareExternal(
    const executable::Executable& diff,
    const fs::path& output,
    const std::optional<fs::path>& expectedOptional,
    int& exitCode,
    std::future<std::string>& stdOut,
    std::future<std::string>& stdErr) const {
    fs::path expected;
    if (expectedOptional) {
        expected = expectedOptional.value();
    } else {
        try {
            expected = output_.value();
        } catch (const std::bad_optional_access&) {
            std::throw_with_nested(std::logic_error{"Invalid test type"});
        }
    }
    const fs::path a = fs::absolute(output);
    const fs::path b = fs::absolute(expected);

    auto callback = [&](process::Result res, std::optional<process::Info>&&) {
        if (res.status == process::status::IL || res.status == process::status::TL) {
            throw std::runtime_error("Diff computation time out");
        }
        exitCode = res.returnCode;
    };

    executable::ExecTaskContext ctx{std::vector{a.string(), b.string()},
                                    boost::this_process::environment(),
                                    fs::current_path(),
                                    std::move(callback)};
    auto task =
        executable::makeTask(diff, std::move(ctx), bp::null, std::ref(stdOut), std::ref(stdErr));

    return task;
}

fs::path Test::root(const fs::path& workspace) {
    auto path = util::split(name_, "/");

    fs::path result = workspace;
    for (auto&& part : path) {
        result /= part;
    }
    return result;
}

namespace {

TestingResult compareInMemory(const fs::path& outputFile, const fs::path& expectedOutput) {
    std::string output = util::file::read(outputFile);
    std::string expected = util::file::read(expectedOutput);

    auto diff = util::diff::Unified(std::move(output), std::move(expected));
    if (diff.empty()) {
        return {NoDiff{}};
    } else {
        return {std::move(diff)};
    }
}

TestingResult compareBlobs(const fs::path& outputFile, const fs::path& expectedOutput) {
    std::ifstream output(outputFile);
    std::ifstream expected(expectedOutput);

    static constexpr size_t regionSize = 64;

    std::array<std::string, 2> regions = {std::string(regionSize, '\0'),
                                          std::string(regionSize, '\0')};

    size_t equalCount = 0;

    auto read = [](std::istream& is, std::string& str) {
        is.read(str.data(), static_cast<std::streamsize>(str.size()));
        str.resize(static_cast<size_t>(is.gcount()));
    };

    while (output && expected) {
        read(output, regions[0]);
        read(expected, regions[1]);
        if (regions[0] != regions[1]) {
            static constexpr std::string_view etc = "...";
            if (equalCount > 0) {
                regions[0].insert(0, etc);
                regions[1].insert(0, etc);
            }

            if (output) {
                regions[0].append(etc);
            }

            if (expected) {
                regions[1].append(etc);
            }

            break;
        }
        ++equalCount;
    }

    if (output || expected) {
        return {BlobDiff(regions[0], regions[1])};
    } else {
        return {NoDiff{}};
    }
}

} // anonymous namespace

} // namespace cacos::test
