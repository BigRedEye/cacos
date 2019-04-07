#if !defined(CACOS_OS_UNIX) && !defined(CACOS_OS_MACOS) && !defined(CACOS_OS_WINDOWS)

#include "cacos/util/terminfo/terminfo.h"

namespace cacos::util::terminfo {

Term get() {
    return Term{80, 24};
}

} // namespace cacos::util::terminfo

#endif // posix
