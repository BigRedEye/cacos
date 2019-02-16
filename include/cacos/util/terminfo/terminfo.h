#include "cacos/util/ints.h"

namespace cacos::util::terminfo {

struct Term {
    ui32 width;
    ui32 height;
    bool tty;
};

Term get();

} // namespace cacos::util::terminfo
