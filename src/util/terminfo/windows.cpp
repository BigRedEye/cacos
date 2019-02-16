#if defined(CACOS_OS_WINDOWS)

#include "cacos/util/terminfo/terminfo.h"

#include <windows.h>
#include <io.h>
#include <cstdio>

namespace cacos::util::terminfo {

Term get() {
    CONSOLE_SCREEN_BUFFER_INFO info;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

    Term result;

    result.width = info.srWindow.Right - info.srWindow.Left + 1;
    result.height = info.srWindow.Bottom - info.srWindow.Top + 1;
    result.tty = _isatty(_fileno(stdin));

    return result;
}

} // namespace cacos::util::terminfo

#endif // posix
