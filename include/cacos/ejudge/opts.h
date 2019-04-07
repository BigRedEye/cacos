#pragma once

#include "cacos/util/ints.h"

#include <optional>
#include <string>

namespace cacos::opts {

struct LoginPassword {
    std::optional<std::string> login;
    std::optional<std::string> password;
};

struct Session {
    std::optional<std::string> ejsid;
    std::optional<std::string> token;
};

struct EjudgeOpts {
    std::string task;
    std::string url;
    i32 contestId;

    LoginPassword login;
    Session session;
};

} // namespace cacos::opts
