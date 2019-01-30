#pragma once

#include "cacos/lang/translator.h"

#include "cacos/task/opts.h"

#include "cacos/executable/executable.h"

namespace cacos::lang {

class Compiler : public Translator {
public:
    Compiler(const cpptoml::table& t, const fs::path& binaryDir_);

    fs::path compile(const fs::path& source, const opts::CompilerOpts& opts = {}) const;
    std::pair<fs::path, executable::ExecTaskPtr> task(
        const fs::path& source,
        const opts::CompilerOpts& opts,
        std::future<std::string>& stdOut,
        std::future<std::string>& stdErr) const;

private:
    fs::path binaryDir_;
};

} // namespace cacos::lang
