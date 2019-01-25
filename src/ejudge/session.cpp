#include "cacos/ejudge/session.h"

#include "cacos/ejudge/html/myhtml.h"

#include "cacos/util/logger.h"

namespace cacos::ejudge {

AuthenticationError::AuthenticationError(const std::string& what)
    : std::runtime_error(what) {
}

SessionError::SessionError(const std::string& what)
    : std::runtime_error(what) {
}

Session::Session(const http::Client& client, const config::Config& config)
    : client_(client)
    , config_(config) {
    prefix_ = util::split(config_.ejudge().url, "?")[0];
    auto& session = config.ejudge().session;
    if (session.ejsid && session.token) {
        Logger::log().print("Found user-defined session (ejsid = {}, token = {})",
            *session.ejsid,
            *session.token);
        token_ = *session.token;
        setCookie(*session.ejsid);
    } else if (loadSession()) {
        Logger::log().print("Found external session");
        return;
    } else {
        Logger::log().print("No external session was found; trying to reauth");
        reauth();
    }
}

void Session::setCookie(std::string_view cookie) const {
    Logger::log().print("Savig cookies: domain = {}, EJSID = {}", domain(), cookie);
    client_.cookie(fmt::format("{}\tFALSE\t/\tFALSE\t0\tEJSID\t{}", domain(), cookie));
}

void Session::saveSession() const {
    Logger::log().print("Savig token: {}", domain(), token_);
    std::ofstream token(config_.file(config::FileType::token));
    fmt::print(token, "{}", token_);
}

bool Session::loadSession() {
    auto file = config_.file(config::FileType::token);
    if (fs::exists(file)) {
        Logger::log().print("Found token at {}", file);
        std::ifstream ifs(file);
        std::getline(ifs, token_);
        return true;
    } else {
        Logger::log().print("Cannot found token at {}", file);
        return false;
    }
}

std::string_view Session::domain() const {
    auto tokens = util::split(prefix_, "/");
    auto view = prefix_;
    if (util::starts_with(tokens[0], "http")) {
        return tokens[2];
    } else {
        return tokens[0];
    }
}

void Session::reauth() {
    Logger::log().print("Trying to reauthenticate");
    InlineVariables vars("config");
    vars.set("contest_id", util::to_string(config_.ejudge().contestId));

    std::string loginPage = client_.post(
        vars.parse(config_.ejudge().url),
        util::join("login=", config_.ejudge().login.login.value(), "&password=", config_.ejudge().login.password.value())
    );

    html::Html page(loginPage);
    html::Collection titles = page.tags("title");
    for (auto node : titles) {
        if (node.child()->text().find("Permission denied") != std::string_view::npos) {
            throw AuthenticationError("Permission deined");
        }
    }
    html::Collection tags = page.attrs("href");
    std::optional<std::string> url;
    for (auto node : tags) {
        url = node.attr("href").value();
    }

    if (!url) {
        throw AuthenticationError("Cannot parse ejudge responce: href not found");
    }

    // https://caos.ejudge.ru/ej/client/main-page/Sa1a9fcc0f19fe4a1?lt=1
    //                                            ^^^^^^^^^^^^^^^^^
    auto tokens = util::split(*url, "/?");
    if (tokens.size() < 2) {
        throw std::runtime_error("Cannot parse ejudge responce: weird link");
    }
    token_ = tokens[tokens.size() - 2];
    if (token_.size() != TOKEN_SIZE) {
        throw AuthenticationError("Cannot parse ejudge responce: weird token");
    }

    saveSession();
    Logger::log().print("Sucessfuly parsed token");
}

std::string Session::buildUrl(std::string_view base) {
    return util::join(prefix_, "/", base, "/", token_);
}

html::Html Session::get(std::string_view base, std::string_view params) {
    for (size_t i = 1; i < MAX_RETRIES; ++i) {
        try {
            return getImpl(base, params);
        } catch (const SessionError& err) {
            Logger::log().print("SessionError {} / {}: {}", i, MAX_RETRIES - 1, err.what());
            reauth();
        } catch (const http::Error& err) {
            Logger::log().print("http::Error {} / {}: {}", i, MAX_RETRIES - 1, err.what());
        }
    }
    return getImpl(base, params);
}

html::Html Session::getImpl(std::string_view base, std::string_view params) {
    std::string url = buildUrl(base);
    if (!params.empty()) {
        url += util::str(params);
    }

    html::Html page(client_.get(url));
    html::Collection titles = page.tags("title");
    for (auto node : titles) {
        if (node.child()->text().find("Permission denied") != std::string_view::npos) {
            throw AuthenticationError("Permission deined");
        }
        if (node.child()->text().find("[]: Error: Invalid session") != std::string_view::npos) {
            throw SessionError("Invalid session");
        }
    }

    return page;
}

}
