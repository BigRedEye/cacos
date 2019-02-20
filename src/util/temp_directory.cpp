#include "cacos/util/temp_directory.h"
#include "cacos/util/string.h"

#include <random>
#include <vector>

namespace cacos::util {

TempDirectory::~TempDirectory() {
    if (!keep_ && fs::exists(path_)) {
        fs::remove_all(path_);
    }
}

const fs::path& TempDirectory::get() const {
    if (path_.empty()) {
        lazyInit();
    }
    return path_;
}

void TempDirectory::keep(bool shouldKeep) {
    keep_ = shouldKeep;
}

namespace {

fs::path findTempRoot(const std::vector<fs::path>& v) {
    for (auto&& path : v) {
        if (fs::exists(path)) {
            return path;
        }
    }
    return fs::temp_directory_path();
}

} // namespace

void TempDirectory::lazyInit() const {
    path_ = findTempRoot({"/var/tmp", "/var/cache"}) / "cacos";

    std::uniform_int_distribution<size_t> ud;
    std::mt19937_64 mt{std::random_device{}()};
    std::string name;

    do {
        name = util::string::to(ud(mt));
    } while (fs::exists(path_ / name));

    path_ /= name;

    fs::create_directories(path_);
}

} // namespace cacos::util
