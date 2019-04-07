#include "cacos/lang/lang.h"

#include "cacos/executable/executable.h"

#include "cacos/config/config.h"

#include "cacos/util/ranges.h"
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

    auto linker = config.get_table("linker");
    if (linker) {
        if (!compiler_) {
            throw config::ConfigError("Found linker without compiler");
        }
        linker_ = Linker(*linker, binaryDir);
    }

    auto exts = config.get_array_of<std::string>("extensions");
    if (!exts) {
        throw config::ConfigError("Cannot find extensions for language");
    }
    std::copy(exts->begin(), exts->end(), std::back_inserter(extensions_));
}

executable::Executable Language::process(const fs::path& source, const opts::CompilerOpts& options)
    const {
    fs::path compiled = source;
    if (compiler_) {
        compiled = compiler_->compile(source, options);
        if (linker_) {
            compiled = linker_->link({compiled}, options);
        }
    }
    executable::Executable result;
    if (interpreter_) {
        result = interpreter_->process(compiled);
    } else {
        result = compiled;
    }
    return result;
}

const std::vector<std::string>& Language::extensions() const {
    return extensions_;
}

util::optional_ref<const Compiler> Language::compiler() const {
    return compiler_ ? util::optional_ref(*compiler_) : util::nullref;
}

util::optional_ref<const Interpreter> Language::interpreter() const {
    return interpreter_ ? util::optional_ref(*interpreter_) : util::nullref;
}

util::optional_ref<const Linker> Language::linker() const {
    return linker_ ? util::optional_ref(*linker_) : util::nullref;
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

executable::Executable LanguageTable::runnable(const opts::ExeOpts& opts) const {
    if (opts.sources.size() == 1) {
        const auto& path = opts.sources[0];
        bool isExecutable = !bp::search_path(path.filename().string()).empty();
        isExecutable =
            isExecutable ||
            !bp::search_path(path.filename().string(), {path.parent_path().string()}).empty();
        if (isExecutable) {
            return path;
        }

        return lang(path).process(path, opts.compiler);
    } else {
        const auto& sources = opts.sources;
        const Linker& linker = lang(sources[0]).linker();

        /* check for common linker */
        for (auto&& path : util::skip(sources, 1)) {
            util::optional_ref<const Linker> curLinker = lang(path).linker();
            if (!curLinker) {
                throw std::runtime_error("Cannot find linker for source " + path.string());
            }
            if (curLinker != linker) {
                throw std::runtime_error(util::string::join(
                    "Mismatched linkers for sources ", sources[0].string(), ", ", path.string()));
            }
        }

        /* compile object files */
        std::vector<fs::path> objects;
        boost::container::stable_vector<std::future<std::string>> stdOuts;
        boost::container::stable_vector<std::future<std::string>> stdErrs;

        executable::ExecPool pool;
        for (auto&& path : sources) {
            const auto& compiler = lang(path).compiler().value();

            stdOuts.emplace_back();
            stdErrs.emplace_back();

            auto [obj, task] = compiler.task(path, opts.compiler, stdOuts.back(), stdErrs.back());
            objects.push_back(obj);
            pool.push(std::move(task));
        }

        pool.run();

        /* link */
        fs::path binary = linker.link(objects, opts.compiler);
        return binary;
    }
}

const Language& LanguageTable::lang(const std::string& ext) const {
    auto it = langByExtension_.find(ext);

    if (it == langByExtension_.end()) {
        throw LanguageError("Cannot determine language");
    }

    return *it->second;
}

const Language& LanguageTable::lang(const fs::path& file) const {
    return lang(file.extension().string());
}

} // namespace cacos::lang
