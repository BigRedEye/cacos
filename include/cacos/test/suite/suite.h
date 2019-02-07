#pragma once

#include "cacos/config/config.h"

#include "cacos/util/util.h"

namespace cacos::test {

class Suite {
public:
    Suite(config::Config& cfg);

private:
    fs::path workspace_;
};

}
