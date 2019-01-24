#pragma once

#include "cacos/util/ints.h"

#include <boost/process.hpp>

#include <filesystem>
#include <chrono>

namespace cacos {

namespace fs = std::filesystem;
namespace bp = boost::process;

using seconds = std::chrono::duration<double>;
using bytes = ui64;

}
