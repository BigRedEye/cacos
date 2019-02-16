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

struct GeneratorOptions {
    using Variables = std::map<std::string, Range<i64>>;

    Type type;
    std::vector<fs::path> generatorSources;
    Variables vars;
    std::string input;
    std::string testName;
    std::vector<std::string> args;
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
