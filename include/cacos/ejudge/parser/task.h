#pragma once

#include "cacos/util/ints.h"

#include <optional>
#include <string>

namespace cacos::ejudge::parser {

struct Result {
    std::string status;
    std::optional<i32> score;
    std::optional<i32> tests;
};

struct Task {
    std::string name;
    i32 id;
    std::optional<Result> result;
};

struct Solution {
    i32 id;
    Result result;
};

}
