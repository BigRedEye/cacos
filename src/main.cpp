#include "cacos/init.h"
#include "cacos/config.h"
#include "cacos/test.h"

#include <cpparg/cpparg.h>

#include <iostream>

#include <cpptoml.h>
#include <string_view>

int main(int argc, const char* argv[]) {
    cpparg::command_parser parser("cacos");
    parser.title("CAOS testing utility");
    parser.command("new").description("Initialize new workspace").handle(cacos::commands::init);
    parser.command("test").description("Manage tests").handle(cacos::commands::test);
    parser.command("run").description("Compile and run").handle(cacos::commands::run);
    return parser.parse(argc, argv);
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
}
