#pragma once

#include "cacos/util/ints.h"

#include <boost/process.hpp>

#include <chrono>
#include <filesystem>

namespace cacos {

namespace fs = std::filesystem;
namespace bp = boost::process;

using seconds = std::chrono::duration<double>;
using bytes = ui64;

} // namespace cacos
