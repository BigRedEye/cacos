#pragma once

#include "cacos/util/util.h"

namespace cacos::http {

namespace request {
enum class Type {
    GET,
    POST,
};

template<Type type>
struct Params;

template<>
struct Params<Type::GET> {
    std::string url;
};

template<>
struct Params<Type::POST> {
    std::string url;
    std::string data;
};
} // namespace request

class Error : public std::runtime_error {
public:
    Error(const std::string& what);
};

class Client {
public:
    Client();
    Client(const fs::path& cookies);

    Client(const Client& other) = delete;
    Client& operator=(const Client& other) = delete;

    Client(Client&& other) = default;
    Client& operator=(Client&& other) = default;

    ~Client();

    std::string request(const request::Params<request::Type::GET>& params) const;
    std::string request(const request::Params<request::Type::POST>& params) const;

    std::string get(const std::string& url) const {
        return request(request::Params<request::Type::GET>{url});
    }

    std::string post(const std::string& url, const std::string& data) const {
        return request(request::Params<request::Type::POST>{url, data});
    }

    void cookie(std::string_view netscape) const;

private:
    class Impl;

    std::unique_ptr<Impl> impl_;
};

} // namespace cacos::http
