#pragma once

#include "cacos/lang/compiler.h"
#include "cacos/lang/interpreter.h"
#include "cacos/lang/linker.h"

#include "cacos/task/opts.h"

#include "cacos/executable/executable.h"

#include "cacos/util/util.h"
#include "cacos/util/optional_ref.h"

#include <boost/container/stable_vector.hpp>

#include <cpptoml.h>

#include <optional>
#include <stdexcept>

namespace cacos::lang {

class LanguageError : public std::runtime_error {
public:
    LanguageError(const std::string& what);
};

class Language {
public:
    Language(const cpptoml::table& config, const fs::path& binaryDir);

    executable::Executable process(const fs::path& sources, const opts::CompilerOpts& opts) const;

    const std::vector<std::string>& extensions() const;

    util::optional_ref<const Compiler> compiler() const;
    util::optional_ref<const Interpreter> interpreter() const;
    util::optional_ref<const Linker> linker() const;

private:
    std::string name_;
    std::vector<std::string> extensions_;
    std::optional<Compiler> compiler_;
    std::optional<Interpreter> interpreter_;
    std::optional<Linker> linker_;
};

class LanguageTable {
public:
    LanguageTable() = default;
    LanguageTable(const cpptoml::table& table, const fs::path& binaryDir);

    executable::Executable runnable(const opts::ExeOpts& opts) const;

private:
    const Language& lang(const std::string& ext) const;
    const Language& lang(const fs::path& file) const;

private:
    std::unordered_map<std::string, Language*> langByExtension_;
    boost::container::stable_vector<Language> langs_;
};

executable::Executable runnable(const fs::path& path);

struct LangOpts {
    lang::LanguageTable langs;
};

} // namespace cacos::lang
