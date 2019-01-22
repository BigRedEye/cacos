#pragma once

#include "cacos/util/util.h"
#include "cacos/options.h"

#include <cpparg/cpparg.h>

namespace cacos {

namespace options {
enum Mask : ui64 {
    WORKSPACE = ui64{1} << 1,
    CONFIG    = ui64{1} << 2,
    LANGS     = ui64{1} << 3,
    EJUDGE    = ui64{1} << 4,
    ALL       = ~ui64{0},
};
}

cpparg::parser& setCommonOptions(cpparg::parser& parser, Options& opts, ui64 mask = options::ALL);

}
