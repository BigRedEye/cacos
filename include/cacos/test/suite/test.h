#pragma once

#include "cacos/executable/executable.h"

#include "cacos/util/util.h"

namespace cacos::test {

enum class Type {
    diff,
    canonical,
};

struct TestingResult {
    std::string diff;
};

struct TaskContext {
    const executable::Executable& exe;
    executable::ExecTaskContext::Callback callback = {};
    fs::path stdOut = {};
    std::optional<fs::path> stdErr = std::nullopt;
};

class Test {
public:
    Test(const fs::path& toml);

    executable::ExecTaskPtr task(TaskContext&& context);

    TestingResult compare(const fs::path& output);
    TestingResult compare(const fs::path& output, const fs::path& expected);

private:
    Type type_;

    fs::path input_;
    std::optional<fs::path> output_ = std::nullopt;

    bp::environment env_ = boost::this_process::environment();
    std::vector<std::string> args_ = {};
};

}
