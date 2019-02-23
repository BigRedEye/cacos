#pragma once

#include "cacos/util/ints.h"

#include <boost/process.hpp>

#include <chrono>

#ifdef __has_include
#if __has_include(<filesystem>)
#include <filesystem>
namespace stdfs = std::filesystem;
#elif __has_include(<expiremental/filesystem>)
#include <experimental/filesystem>
namespace stdfs = std::experimental::filesystem;
#elif __has_include(<boost/filesystem.hpp>)
#include <boost/filesystem.hpp>
namespace stdfs = boost::filesystem;
#else
#error "Missing <filesystem>"
#endif
#else
#include <filesystem>
namespace stdfs = std::filesystem;
#endif // __has_include

namespace cacos {

namespace fs = stdfs;
namespace bp = boost::process;

using seconds = std::chrono::duration<double>;
using bytes = ui64;

} // namespace cacos
