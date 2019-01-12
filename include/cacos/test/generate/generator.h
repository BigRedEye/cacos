#pragma once

#include "cacos/options.h"

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

struct GeneratorOptions : public Options {
    using Variables = std::map<std::string, Range<int>>;

    std::string generator;
    std::vector<std::string> args;
    std::string input;
    Variables vars;
};

class Generator {
public:
    Generator(const GeneratorOptions& opts);
    Generator(GeneratorOptions&& opts);

    void run();

private:
    using VarsIterator = GeneratorOptions::Variables::iterator;

    void traverse(
        VarsIterator it,
        InlineVariables& vars,
        const std::function<void(const InlineVariables& vars)>& callback);

private:
    GeneratorOptions opts_;
};

} // namespace cacos::test
