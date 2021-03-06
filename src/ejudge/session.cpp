#include "cacos/ejudge/session.h"

#include "cacos/ejudge/html/myhtml.h"

#include "cacos/util/logger.h"

#include <string>

namespace cacos::ejudge {

AuthenticationError::AuthenticationError(const std::string& what)
    : std::runtime_error(what) {
}

SessionError::SessionError(const std::string& what)
    : std::runtime_error(what) {
}

class Session::Cache {
public:
    struct Response {
        std::string data;
        std::chrono::high_resolution_clock::time_point expiration;
    };

    std::optional<Response*> get(const std::string& req) {
        auto it = cache_.find(req);
        if (it == cache_.end()) {
            return std::nullopt;
        } else {
            return &it->second;
        }
    }

    std::string_view set(const std::string& key, const std::string& value) {
        auto [it, inserted] = cache_.emplace(
            key, Response{value, std::chrono::high_resolution_clock::now() + CACHE_EXPIRATION});
        return it->second.data;
    }

private:
    std::unordered_map<std::string, Response> cache_;
};

Session::Session(const config::Config& config)
    : config_(config)
    , client_(config.file(config::FileType::cookies))
    , prefix_(config_.ejudge().url)
    , cache_(std::make_unique<Cache>()) {
    auto& session = config.ejudge().session;
    if (session.ejsid && session.token) {
        log::log().print(
            "Found user-defined session (ejsid = {}, token = {})", *session.ejsid, *session.token);
        token_ = *session.token;
        setCookie(*session.ejsid);
    } else if (loadSession()) {
        log::log().print("Found cached session");
        return;
    } else {
        log::log().print("No cached session was found; trying to reauth");
        reauth();
    }
}

Session::~Session() {
}

void Session::setCookie(std::string_view cookie) const {
    log::log().print("Saving cookies: EJSID = {}", cookie);
    client_.cookie(fmt::format("{}\tFALSE\t/\tFALSE\t0\tEJSID\t{}", domain(), cookie));
}

void Session::saveSession() const {
    log::log().print("Saving token: {}", domain(), token_);
    std::ofstream token(config_.file(config::FileType::token));
    fmt::print(token, "{}", token_);
}

bool Session::loadSession() {
    auto file = config_.file(config::FileType::token);
    if (fs::exists(file)) {
        log::log().print("Found token at {}", file);
        std::ifstream ifs(file);
        std::getline(ifs, token_);
        return true;
    } else {
        log::log().print("Cannot find token at {}", file);
        return false;
    }
}

std::string_view Session::domain() const {
    auto tokens = util::split(prefix_, "/");
    auto view = prefix_;
    if (util::string::starts(tokens[0], "http")) {
        return tokens[2]; /* "http:" "" "caos.ejudge.ru" ... */
    } else {
        return tokens[0];
    }
}

void Session::reauth() {
    log::log().print("Trying to reauthenticate");
    std::string clientUrl =
        util::string::join(prefix_, "?contest_id=", util::string::to(config_.ejudge().contestId));

    std::string loginPage = client_.post(
        clientUrl,
        util::string::join(
            "login=",
            config_.ejudge().login.login.value(),
            "&password=",
            config_.ejudge().login.password.value()));
    log::debug().print("Login page:\n{}", loginPage);

    html::Html page(loginPage);
    html::Collection titles = page.tags("title");
    for (auto node : titles) {
        if (node.child()->text().find("Permission denied") != std::string_view::npos) {
            throw AuthenticationError("Permission denied");
        }
    }

    html::Collection tags = page.attrs("href");
    std::optional<std::string> url;
    for (auto node : tags) {
        url = node.attr("href").value();
    }

    if (!url) {
        throw AuthenticationError("Cannot parse ejudge response: href not found");
    }

    // https://caos.ejudge.ru/ej/client/main-page/Sa1a9fcc0f19fe4a1?lt=1
    //                                            ^^^^^^^^^^^^^^^^^
    auto tokens = util::split(*url, "/?&");
    if (tokens.size() < 2) {
        throw std::runtime_error("Cannot parse ejudge response: weird link");
    }

    for (auto token : tokens) {
        if (token.size() == TOKEN_SIZE && util::string::starts(token, "S")) {
            token_ = token;
        } else if (util::string::starts(token, "SID=")) {
            throw AuthenticationError("Unsupported ejudge url");
        }
    }

    if (token_.empty()) {
        throw AuthenticationError("Cannot parse ejudge response: weird token");
    }

    saveSession();
    log::log().print("Successfully parsed token: {}", token_);
}

std::string Session::buildUrl(std::string_view base) {
    return util::string::join(prefix_, "/", base, "/", token_);
}

html::Html Session::getPage(std::string_view base, std::string_view params) {
    return getter(
        base, params, [this](auto base, auto params) { return getImpl(base, params).second; });
}

std::string_view Session::getRaw(std::string_view base, std::string_view params) {
    return getter(
        base, params, [this](auto base, auto params) { return getImpl(base, params).first; });
}

std::string_view Session::rawGetter(std::string_view base, std::string_view params) {
    std::string url = buildUrl(base);
    if (!params.empty()) {
        url += "?" + util::str(params);
    }
    if (auto res = cache_->get(url)) {
        if ((*res)->expiration > std::chrono::high_resolution_clock::now()) {
            log::debug().print("Found cached page: url {}", url);
            return (*res)->data;
        }
    }
    std::string result = client_.get(url);
    log::debug().print("Get page impl: url {}, page:\n{}", url, result);
    return cache_->set(url, result);
}

std::pair<std::string_view, html::Html> Session::getImpl(
    std::string_view base,
    std::string_view params) {
    std::string_view raw = rawGetter(base, params);
    html::Html page(raw);
    html::Collection titles = page.tags("title");
    for (auto node : titles) {
        auto throwIfContains = [&](std::string_view s, auto dummy) {
            using Error = std::decay_t<decltype(dummy)>;
            if (node.child()->text().find(s) != std::string_view::npos) {
                throw Error(util::str(s));
            }
        };
        throwIfContains("Permission denied", AuthenticationError{""});
        throwIfContains("[]: Error: Invalid session", SessionError{""});
        throwIfContains("[]: Error: Invalid contest", SessionError{""});
    }

    return {raw, std::move(page)};
}

} // namespace cacos::ejudge
