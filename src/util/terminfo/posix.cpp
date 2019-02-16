#if defined(CACOS_OS_UNIX) || defined(CACOS_OS_MACOS)

#include "cacos/util/terminfo/terminfo.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>

namespace cacos::util::terminfo {

Term get() {
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    Term result;

    result.width = size.ws_col;
    result.height = size.ws_row;
    result.tty = isatty(fileno(stdin));

    return result;
}

} // namespace cacos::util::terminfo

#endif // posix
