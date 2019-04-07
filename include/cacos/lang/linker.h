#pragma once

#include "cacos/lang/translator.h"

#include "cacos/task/opts.h"

namespace cacos::lang {

class Linker : public Translator {
public:
    Linker(const cpptoml::table& t, const fs::path& binaryDir);

    fs::path link(const std::vector<fs::path>& objs, const opts::CompilerOpts& options) const;

private:
    fs::path binaryDir_;
};

} // namespace cacos::lang
