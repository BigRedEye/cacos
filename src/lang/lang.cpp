#include "cacos/lang/lang.h"

#include "cacos/config/config.h"

#include "cacos/util/util.h"

namespace cacos::lang {

LanguageError::LanguageError(const std::string& what)
    : std::runtime_error(what) {
}

Language::Language(const cpptoml::table& config, const fs::path& binaryDir) {
    auto compiler = config.get_table("compiler");
    if (compiler) {
        compiler_ = Compiler(*compiler, binaryDir);
    }

    auto interpreter = config.get_table("interpreter");
    if (interpreter) {
        interpreter_ = Interpreter(*interpreter);
    }

    auto exts = config.get_array_of<std::string>("extensions");
    if (!exts) {
        throw config::ConfigError("Cannot find extensions for language");
    }
    std::copy(exts->begin(), exts->end(), std::back_inserter(extensions_));
}

executable::Executable Language::process(const fs::path& source) const {
    fs::path compiled = source;
    if (compiler_) {
        compiled = compiler_->process(source).path();
    }
    executable::Executable result = compiled;
    if (interpreter_) {
        result = interpreter_->process(result.path());
    }
    return result;
}

const std::vector<std::string>& Language::extensions() const {
    return extensions_;
}

LanguageTable::LanguageTable(const cpptoml::table& table, const fs::path& binaryDir) {
    auto langs = table.get_table_array("lang");
    if (!langs) {
        throw config::ConfigError("Cannot find languages table");
    }
    for (auto& lang : *langs) {
        langs_.push_back(Language(*lang, binaryDir));
        for (const auto& ext : langs_.back().extensions()) {
            langByExtension_[ext] = &langs_.back();
        }
    }
}

executable::Executable LanguageTable::runnable(const fs::path& path) const {
    bool isExecutable = !bp::search_path(path.filename().string()).empty();
    isExecutable =
        isExecutable || !bp::search_path(path.filename().string(), {path.parent_path().string()}).empty();
    if (isExecutable) {
        return path;
    }

    std::string extension = path.extension();
    auto it = langByExtension_.find(extension);

    if (it == langByExtension_.end()) {
        throw LanguageError("Cannot determine language");
    }

    return it->second->process(path);
}

} // namespace cacos::lang
