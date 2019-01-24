#include "cacos/cacos.h"
#include "cacos/util/logger.h"

#include <stdexcept>

void printException(const std::exception& e, size_t depth = 1) noexcept {
    cacos::Logger::error() << std::string(depth, '\t') << e.what();
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& nested) {
        printException(nested, depth + 1);
    } catch (...) {
    }
}

int main(int argc, const char* argv[]) {
    try {
        return cacos::main(argc, argv);
    } catch (const std::exception& e) {
        cacos::Logger::error() << "Got exception:";
        printException(e);
        return 1;
    }
}
