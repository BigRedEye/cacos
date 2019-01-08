#include "cacos/init.h"
#include "cacos/config.h"
#include "cacos/common_args.h"
#include "cacos/options.h"

#include <cpparg/cpparg.h>
#include <termcolor/termcolor.hpp>

#include <string>
#include <fstream>
#include <filesystem>

namespace cacos::commands {

int init(int argc, const char* argv[]) {
    try {
        fs::path dir;

        cpparg::parser parser("cacos new");
        parser.title("Initialize new workspace");

        Options opts;
        setCommonOptions(parser, opts, options::WORKSPACE);

        parser.parse(argc, argv);

        std::vector<std::string_view> required_dirs = { ".bin", ".test" };

        for (auto sv : required_dirs) {
            if (!fs::create_directories(opts.workspace / sv)) {
                throw std::runtime_error("Cannot create directory");
            }
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << termcolor::red << ex.what() << std::endl;
        return 1;
    }
}

}
