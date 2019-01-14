#include "cacos/lang/lang.h"

#include "cacos/config.h"

#include "cacos/util/util.h"

namespace cacos::lang {

LanguageError::LanguageError(const std::string& what)
    : std::runtime_error(what)
{}

Language::Language(const cpptoml::table& config, const fs::path& binaryDir) {
    auto compiler = config.get_table("compiler");
    if (compiler) {
        compiler_ = Compiler(*compiler, binaryDir);
    }

    auto interpreter = config.get_table("interpreter");
    if (interpreter) {
        interpreter_ = Interpreter(*interpreter);
    }
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

executable::Executable LanguageTable::runnable(const fs::path& path) {
    bool isExecutable = !bp::search_path(path.string()).empty();
    isExecutable = isExecutable || !bp::search_path(path.string(), { path.parent_path().string() }).empty();
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

/*
    auto config = cpptoml::parse_file("/home/sergey/dev/cacos/cacos.toml");
    auto langs = config->get_table_array("lang");
    for (auto& lang : *langs) {
        auto name = lang->get_as<std::string>("name").value_or("name");
        std::cout << name << std::endl;
        auto compiler = lang->get_table("compiler");
        if (compiler) {
            std::cout << "  " << "has compiler" << std::endl;
            std::vector<std::string> flags = compiler->get_array_of<std::string>("flags")
                .value_or<std::vector<std::string>>({});
            auto release_flags = compiler->get_array_of<std::string>("release")
                .value_or<std::vector<std::string>>({});
            for (auto& s : release_flags) {
                flags.push_back(s);
            }
            for (auto& s : flags) {
                if (s == "@source") {
                    s = "main.cpp";
                } else if (s == "@binary") {
                    s = "main.exe";
                }
            }
            std::cout << "  $ " << compiler->get_as<std::string>("exe").value_or("compiler") << " ";
            for (auto& s : flags) {
                std::cout << s << ' ';
            }
            std::cout << std::endl;
        }
        auto interpreter = lang->get_table("interpreter");
        if (interpreter) {
            std::cout << "  " << "has interpreter" << std::endl;
        }
        auto extensions = lang->get_array_of<std::string>("extensions");
        if (extensions) {
            std::cout << "extensions: [";
            for (auto& s : *extensions) {
                std::cout << ' ' << s << ',';
            }
            std::cout << " ]\n";
        }
        std::cout << '\n';
    }
*/
