#pragma once

#include "cacos/test/suite/test.h"

#include "cacos/config/config.h"

#include "cacos/executable/executable.h"

#include "cacos/util/util.h"

#include <unordered_map>
#include <vector>

namespace cacos::test {

struct RunOpts {
    process::Limits limits;
    bool printInfo{false};
    bool allowNonZeroReturnCodes{false};
    size_t workers{std::thread::hardware_concurrency()};
};

class Suite {
public:
    Suite(const config::Config& cfg, std::string_view prefix = "");

    void run(
        const RunOpts& opts,
        const executable::Executable& exe,
        util::optional_ref<const executable::Executable> checker = util::nullref);

private:
    fs::path workspace_;
    const config::Config& config_;
    std::unordered_map<Type, std::vector<Test>> tests_;
};

} // namespace cacos::test
