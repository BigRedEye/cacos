#ifdef CACOS_HAS_CURL

#include "cacos/ejudge/http/client.h"

#include "cacos/util/string.h"
#include "cacos/util/logger.h"

#include "curl/curl.h"

#include <stdexcept>

#include <iostream>

namespace cacos::http {

class CurlLoader {
public:
    CurlLoader() {
        ++loads_;
        if (loads_ == 1) {
            curl_global_init(CURL_GLOBAL_ALL);
        }
    }

    ~CurlLoader() {
        --loads_;
        if (loads_ == 0) {
            curl_global_cleanup();
        }
    }

private:
    static int loads_;
};

int CurlLoader::loads_ = 0;

size_t curlWriteToString(void* contents, size_t size, size_t nmemb, void* userp) {
    std::string* buf = reinterpret_cast<std::string*>(userp);
    buf->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

Error::Error(const std::string& what)
    : std::runtime_error(what) {
}

class Client::Impl : public CurlLoader {
public:
    Impl() {
        curl_ = curl_easy_init();
        if (!curl_) {
            throw Error("Cannot initialize curl: curl_easy_init returned nullptr");
        }
    }

    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;

    Impl(Impl&& other) {
        *this = std::move(other);
    }

    Impl& operator=(Impl&& other) {
        std::swap(curl_, other.curl_);
        return *this;
    }

    ~Impl() {
        curl_easy_cleanup(curl_);
    }

    void cookies(const fs::path& path) {
        cookies_ = path;
    }

    template<request::Type type>
    std::string request(const request::Params<type>& params) const {
        Logger::debug().print("Perforing curl request: url = {}", params.url);
        curl_easy_reset(curl_);

        std::string result;

        if (!cookies_.empty()) {
            curl_easy_setopt(curl_, CURLOPT_COOKIEJAR, cookies_.c_str());
            curl_easy_setopt(curl_, CURLOPT_COOKIEFILE, cookies_.c_str());
        }

        for (auto&& c : overridenCookies_) {
            curl_easy_setopt(curl_, CURLOPT_COOKIE, c.data());
        }

        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, curlWriteToString);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &result);
        curl_easy_setopt(curl_, CURLOPT_URL, params.url.data());
        if constexpr (type == request::Type::POST) {
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, params.data.data());
        }
        curl_easy_setopt(curl_, CURLOPT_COOKIE, "FLUSH");

        perform();

        return result;
    }

    void cookie(std::string_view kv) {
        overridenCookies_.push_back(util::str(kv));
    }

private:
    void perform() const {
        CURLcode err = curl_easy_perform(curl_);
        if (err != CURLE_OK) {
            throw Error(util::join("Cannot perform curl request: ", curl_easy_strerror(err)));
        }
    }

private:
    CURL* curl_ = nullptr;
    fs::path cookies_;
    std::vector<std::string> overridenCookies_;
};

Client::Client()
    : impl_(std::make_unique<Impl>()) {
}

Client::Client(const fs::path& cookies)
    : Client() {
    impl_->cookies(cookies);
}

Client::~Client() {
}

std::string Client::request(const request::Params<request::Type::GET>& params) const {
    return impl_->request(params);
}

std::string Client::request(const request::Params<request::Type::POST>& params) const {
    return impl_->request(params);
}

void Client::cookie(std::string_view netscape) const {
    return impl_->cookie(netscape);
}

}

#endif // CACOS_HAS_CURL
