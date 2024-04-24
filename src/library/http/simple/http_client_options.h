#pragma once

#include <ydb-cpp-sdk/util/datetime/base.h>
#include <src/library/string_utils/url/url.h>

class TSimpleHttpClientOptions {
    using TSelf = TSimpleHttpClientOptions;

public:
    TSimpleHttpClientOptions() = default;

    explicit TSimpleHttpClientOptions(std::string_view url) {
        std::string_view scheme, host;
        GetSchemeHostAndPort(url, scheme, host, Port_);
        Host_ = url.substr(0, scheme.size() + host.size());
    }

    TSelf& Host(std::string_view host) {
        Host_ = host;
        return *this;
    }

    const std::string& Host() const noexcept {
        return Host_;
    }

    TSelf& Port(ui16 port) {
        Port_ = port;
        return *this;
    }

    ui16 Port() const noexcept {
        return Port_;
    }

    TSelf& SocketTimeout(TDuration timeout) {
        SocketTimeout_ = timeout;
        return *this;
    }

    TDuration SocketTimeout() const noexcept {
        return SocketTimeout_;
    }

    TSelf& ConnectTimeout(TDuration timeout) {
        ConnectTimeout_ = timeout;
        return *this;
    }

    TDuration ConnectTimeout() const noexcept {
        return ConnectTimeout_;
    }

private:
    std::string Host_;
    ui16 Port_;
    TDuration SocketTimeout_ = TDuration::Seconds(5);
    TDuration ConnectTimeout_ = TDuration::Seconds(30);
};
