#pragma once

#include "cacos/config/config.h"

#include "cacos/ejudge/http/client.h"
#include "cacos/ejudge/html/myhtml.h"

#include <string_view>

namespace cacos::ejudge {

class AuthenticationError : public std::runtime_error {
public:
    AuthenticationError(const std::string& what);
};

class SessionError : public std::runtime_error {
public:
    SessionError(const std::string& what);
};

class Session {
public:
    Session(const http::Client& client, const config::Config& config);

    std::string buildUrl(std::string_view base);
    html::Html get(std::string_view base, std::string_view params = "");

private:
    void reauth();
    void setCookie(std::string_view cookie) const;
    void saveSession() const;
    bool loadSession();

    std::string_view domain() const;
    html::Html getImpl(std::string_view base, std::string_view params = "");

private:
    static constexpr std::size_t TOKEN_SIZE = 17;
    static constexpr std::size_t MAX_RETRIES = 5;

    const http::Client& client_;
    const config::Config& config_;
    std::string prefix_;
    std::string token_;
};

}
