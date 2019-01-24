#pragma once

#include "cacos/util/util.h"

#include <optional>

namespace cacos {

namespace ejudge {

struct LoginPassword {
    std::optional<std::string> login;
    std::optional<std::string> password;
};

struct LoginCookie {
    /*
     * EJSID=cookie
     */
    std::optional<std::string> cookie;

    /*
     * https://caos.ejudge.ru/ej/client/main-page/TOKEN?lt=1
     *                                            ^^^^^
     */
    std::optional<std::string> token;
};

struct Ejudge {
    std::string url;
    i32 contestId;

    LoginPassword loginPassword;
    LoginCookie loginCookie;
};

}

struct Options {
    fs::path workspace;
    fs::path config;
    fs::path langs;

    ejudge::Ejudge ejudge;
};

}
