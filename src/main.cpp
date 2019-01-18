#include "cacos/cacos.h"
#include "cacos/util/logger.h"

#include <stdexcept>

int main(int argc, const char* argv[]) {
    try {
        return cacos::main(argc, argv);
    } catch (const std::exception& e) {
        cacos::Logger::fatal() << "Got exception: " << e.what();
    }
}
