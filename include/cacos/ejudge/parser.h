#pragma once

#include "cacos/config.h"

namespace cacos::ejudge {

class Parser {
public:
    Parser(const config::Config& cfg);

private:
    std::string cookie;
};

}
