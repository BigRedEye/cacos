#include "cacos/cacos.h"
#include "cacos/util/logger.h"
#include "cacos/util/util.h"

#include <termcolor/termcolor.hpp>

#include <boost/stacktrace.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include <stdexcept>

#ifdef CACOS_OS_WINDOWS
#include "windows.h"
#endif // CACOS_OS_WINDOWS

/* based on https://en.cppreference.com/w/cpp/error/throw_with_nested */
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

[[noreturn]] void terminate_handler() {
    std::cerr << termcolor::reset << termcolor::red << "Terminate called ";
    if (std::current_exception()) {
        std::cerr << "after throwing an exception:\n"
                  << boost::current_exception_diagnostic_information() << '\n';
    } else {
        std::cerr << "without active exception\n";
    }
    try {
        cacos::fs::path path = cacos::fs::temp_directory_path() / "cacos.stacktrace";
        std::ofstream ofs(path);
        ofs << boost::stacktrace::stacktrace();
        std::cerr << "Stacktrace dumped to " << path.string() << '\n';
    } catch (...) {
        std::cerr << "Stacktrace dumping failed\n";
    }
    std::cerr << termcolor::reset;
    std::abort();
}

int main(int argc, const char* argv[]) {
#ifdef CACOS_OS_WINDOWS
    ConsoleCodePageSetter cp;
#endif // CACOS_OS_WINDOWS

    std::set_terminate(terminate_handler);

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
