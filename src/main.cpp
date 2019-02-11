#include "cacos/cacos.h"
#include "cacos/util/logger.h"

#include <stdexcept>

#ifdef CACOS_OS_WINDOWS
#include "windows.h"
#endif // CACOS_OS_WINDOWS

void printException(const std::exception& e, size_t depth = 1) noexcept {
    cacos::log::error() << std::string(depth, '\t') << e.what();
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& nested) {
        printException(nested, depth + 1);
    } catch (...) {
    }
}

#ifdef CACOS_OS_WINDOWS

class ConsoleCodePageSetter {
public:
    ConsoleCodePageSetter()
        : inputCodePage_{GetConsoleCP()}
        , outputCodePage_{GetConsoleOutputCP()} {
        set(CP_UTF8, CP_UTF8);
    }

    ~ConsoleCodePageSetter() {
        set(inputCodePage_, outputCodePage_);
    }

private:
    void set(UINT input, UINT output) {
        SetConsoleCP(input);
        SetConsoleOutputCP(output);
    }

private:
    UINT inputCodePage_;
    UINT outputCodePage_;
};

#endif // CACOS_OS_WINDOWS

int main(int argc, const char* argv[]) {
#ifdef CACOS_OS_WINDOWS
    ConsoleCodePageSetter cp;
#endif // CACOS_OS_WINDOWS

#if CACOS_DEBUG
    return cacos::main(argc, argv);
#else
    try {
        return cacos::main(argc, argv);
    } catch (const std::exception& e) {
        cacos::log::error() << "Got exception:";
        printException(e);
        return 1;
    }
#endif
}
