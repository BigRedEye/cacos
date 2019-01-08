#pragma once

#include "cacos/util.h"
#include "cacos/executable/flags.h"

namespace cacos::executable {

class Executable {
public:
    Executable(const fs::path& exe);

    template<typename ...Args>
    bp::child run(
        const std::unordered_map<std::string, std::string>& vars,
        Args... args) {
        bp::child result(bp::exe = executable_.string(), bp::args += flags_.build(vars), args...);
        return result;
    }

private:
    fs::path executable_;
    Flags flags_;
};

}
