#pragma once

#include "cacos/config/config.h"

#include "cacos/ejudge/html/myhtml.h"
#include "cacos/ejudge/http/client.h"

#include "cacos/util/logger.h"

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
    Session(const config::Config& config);
    ~Session();

    std::string buildUrl(std::string_view base);
    html::Html getPage(std::string_view base, std::string_view params = "");
    std::string_view getRaw(std::string_view base, std::string_view params = "");

private:
    void reauth();
    void setCookie(std::string_view cookie) const;
    void saveSession() const;
    bool loadSession();

    std::string_view domain() const;

    html::Html getPageImpl(std::string_view base, std::string_view params = "");
    std::string_view getImpl(std::string_view base, std::string_view params);

    template<typename Callback>
    auto getter(std::string_view base, std::string_view params, Callback&& callback) {
        for (ui32 i = 1; i < MAX_RETRIES; ++i) {
            try {
                return callback(base, params);
            } catch (const SessionError& err) {
                log::log().print("SessionError {} / {}: {}", i, MAX_RETRIES - 1, err.what());
                reauth();
            } catch (const http::Error& err) {
                log::log().print("http::Error {} / {}: {}", i, MAX_RETRIES - 1, err.what());
            }
            std::this_thread::sleep_for(RETRY_DELAY);
        }
        return callback(base, params);
    }

private:
    /*
     *  https://caos.ejudge.ru/ej/client/main-page/Sa3b04b0224c2d708?lt=1
     *                                        len('.................') == 17
     */
    static constexpr ui32 TOKEN_SIZE = 17;
    static constexpr ui32 MAX_RETRIES = 5;
    static constexpr auto RETRY_DELAY = std::chrono::milliseconds(100);

    const config::Config& config_;
    http::Client client_;
    std::string prefix_;
    std::string token_;

    class Cache;
    mutable std::unique_ptr<Cache> cache_;
};

} // namespace cacos::ejudge
