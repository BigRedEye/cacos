#pragma once

#include "cacos/util/util.h"

namespace cacos::process {

struct Limits {
    template<typename T>
    static constexpr T unlimited = T{0};

    bytes ml = unlimited<bytes>;
    seconds cpu = unlimited<seconds>;
    seconds real = unlimited<seconds>;
};

}
