#pragma once

#include "cacos/ejudge/html/myhtml.h"

#include <string_view>

namespace cacos::html {

class Parser {
public:
    Html parse(std::string_view html) const;
};

}
