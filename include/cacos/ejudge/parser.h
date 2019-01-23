#pragma once

#include "cacos/options.h"

namespace cacos::ejudge {

class Parser {
public:
    Parser(const Options& opts);

private:
    std::string cookie;
};

}
