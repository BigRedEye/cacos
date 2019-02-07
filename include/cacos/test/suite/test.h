#pragma once

#include "cacos/executable/executable.h"

#include "cacos/util/util.h"

#include "cacos/util/diff/unified.h"

#include <array>
#include <variant>

namespace cacos::test {

enum class Type {
    diff,
    canonical,
};

std::string serialize(Type type);

class BlobDiff {
public:
    BlobDiff() = default;
    BlobDiff(const std::string& left, const std::string& right);

    const std::string& left() const;
    const std::string& right() const;

private:
    std::string left_;
    std::string right_;
};

struct NoDiff {};

struct TestingResult {
    std::variant<NoDiff, BlobDiff, util::diff::Unified> diff;
};

struct TaskContext {
    const executable::Executable& exe;
    executable::ExecTaskContext::Callback callback = {};
    fs::path stdOut = {};
    std::optional<fs::path> stdErr = std::nullopt;
};

class Test {
public:
    Test() = default;
    Test(const fs::path& base, const fs::path& toml);

    void serialize(const fs::path& workspace);

    Test& type(Type type);
    Test& name(const std::string& name);
    Test& input(const fs::path& stdIn);
    Test& output(const fs::path& stdOut);
    Test& env(const bp::environment& env);
    Test& args(const std::vector<std::string>& args);

    Type type() const;
    const std::string& name() const;
    int returnCode() const;

    executable::ExecTaskPtr task(TaskContext&& context) const;

    TestingResult compare(const fs::path& output) const;
    TestingResult compare(const fs::path& output, const fs::path& expected) const;

    static constexpr std::string_view CONFIG_FILE = "test.toml";
    static constexpr std::string_view INPUT_FILE = "input";
    static constexpr std::string_view OUTPUT_FILE = "output";

private:
    fs::path root(const fs::path& workspace);

private:
    Type type_;
    std::string name_;

    fs::path input_;
    std::optional<fs::path> output_ = std::nullopt;
    std::optional<bp::environment> env_ = std::nullopt;
    std::vector<std::string> args_ = {};
};

} // namespace cacos::test
