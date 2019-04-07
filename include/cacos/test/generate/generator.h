#pragma once

#include "cacos/config/config.h"

#include "cacos/test/suite/test.h"

#include "cacos/util/inline_variables.h"

#include <functional>
#include <map>
#include <string>

namespace cacos::test {

template<typename T>
struct Range {
    T from;
    T to;
    T step;
};

struct IO {
    std::string input;
    std::string output;
};

struct GeneratorOptions {
    using Variables = std::map<std::string, Range<i64>>;

    Type type = Type::canonical;
    std::string name;

    Variables vars;

    std::vector<fs::path> generatorSources;
    std::vector<std::string> args;
    bp::environment env = boost::this_process::environment();

    IO genIO;
    IO testIO;

    std::vector<std::string> testArgs;

    bool force = false;
};

class Generator {
public:
    Generator(const config::Config& cfg, const GeneratorOptions& opts);

    void run();

private:
    using VarsIterator = GeneratorOptions::Variables::iterator;

    void traverse(
        VarsIterator it,
        InlineVariables& vars,
        const std::function<void(const InlineVariables& vars)>& callback);

private:
    const config::Config& config_;
    GeneratorOptions opts_;
};

} // namespace cacos::test
