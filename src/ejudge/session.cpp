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

class Session::Cache {
public:
    struct Responce {
        std::string data;
        std::chrono::high_resolution_clock::time_point expiration;
    };

    std::optional<Responce*> get(const std::string& req) {
        auto it = cache_.find(req);
        if (it == cache_.end()) {
            return std::nullopt;
        } else {
            return &it->second;
        }
    }

    std::string_view set(const std::string& key, const std::string& value) {
        auto [it, inserted] = cache_.emplace(key, Responce{value, std::chrono::high_resolution_clock::now() + std::chrono::seconds(1)});
        return it->second.data;
    }

private:
    std::unordered_map<std::string, Responce> cache_;
};

Session::Session(const config::Config& config)
    : config_(config)
    , client_(config.file(config::FileType::cookies))
    , cache_(std::make_unique<Cache>())
{
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

Session::~Session() {
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
        Logger::log().print("Cannot find token at {}", file);
        return false;
    }
}

std::string_view Session::domain() const {
    auto tokens = util::split(prefix_, "/");
    auto view = prefix_;
    if (util::starts_with(tokens[0], "http")) {
        return tokens[2]; /* "http:" "" "caos.ejudge.ru" ... */
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

html::Html Session::getPage(std::string_view base, std::string_view params) {
    return getter(base, params, [this](auto base, auto params) {
        return getPageImpl(base, params);
    });
}

std::string_view Session::getRaw(std::string_view base, std::string_view params) {
    return getter(base, params, [this](auto base, auto params) {
        return getImpl(base, params);
    });
}

std::string_view Session::getImpl(std::string_view base, std::string_view params) {
    std::string url = buildUrl(base);
    if (!params.empty()) {
        url += "?" + util::str(params);
    }
    if (auto res = cache_->get(url)) {
        if ((*res)->expiration > std::chrono::high_resolution_clock::now()) {
            return (*res)->data;
        }
    }
    std::string result = client_.get(url);
    return cache_->set(url, result);
}

html::Html Session::getPageImpl(std::string_view base, std::string_view params) {
    html::Html page(getImpl(base, params));
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
