#pragma once

#include "cacos/util/util.h"

namespace cacos::util {

class TempDirectory {
public:
    TempDirectory() = default;

    TempDirectory(const TempDirectory& other) = delete;
    TempDirectory(TempDirectory&& other) = default;

    TempDirectory& operator=(const TempDirectory& other) = delete;
    TempDirectory& operator=(TempDirectory&& other) = default;

    ~TempDirectory();

    const fs::path& get() const;
    void keep(bool);

private:
    void lazyInit() const;

private:
    mutable fs::path path_;
    bool keep_ = false;
};

} // namespace cacos::util
