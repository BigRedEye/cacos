#pragma once

#include "cacos/util/util.h"
#include "cacos/options.h"

#include <cpparg/cpparg.h>

#include <numeric>

namespace cacos {

namespace options {
enum Mask : ui64 {
    WORKSPACE = 0x0001,
    CONFIG    = 0x0002,
    LANGS     = 0x0004,
    ALL       = std::numeric_limits<ui64>::max(),
};
}

cpparg::parser& setCommonOptions(cpparg::parser& parser, Options& opts, options::Mask mask = options::ALL);

}
