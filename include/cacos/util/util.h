#pragma once

#include <boost/process.hpp>

#include <filesystem>
#include <cstdint>

#include <cpptoml.h>

namespace cacos {

namespace fs = std::filesystem;
namespace bp = boost::process;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using ui8 = uint8_t;
using ui16 = uint16_t;
using ui32 = uint32_t;
using ui64 = uint64_t;

using seconds = std::chrono::duration<double>;
using bytes = ui64;

}
