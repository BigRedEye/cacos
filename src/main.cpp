#include <cpparg/cpparg.h>
#include <string>

int main(int argc, const char* argv[]) {
    cpparg::parser parser("cacos");
    std::string mode;
    parser.add('i', "init").store(mode);
    parser.parse(argc, argv);
}
