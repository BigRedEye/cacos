#pragma once

#include "cacos/util/ints.h"

#include <string>
#include <optional>

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

}
