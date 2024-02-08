#pragma once

#include <library/cpp/digest/lower_case/hash_ops.h>

#include <util/str_stl.h>

#include <util/system/defaults.h>
#include <util/string/cast.h>
#include <library/cpp/cgiparam/cgiparam.h>
#include <util/network/address.h>
#include <util/network/socket.h>
#include <util/generic/hash.h>
#include <util/system/yassert.h>
#include <util/generic/string.h>
#include <util/datetime/base.h>

#include <util/generic/maybe.h>

using THttpHeadersContainer = THashMap<std::string, std::string, TCIOps, TCIOps>;

class TBaseServerRequestData {
public:
    TBaseServerRequestData(SOCKET s = INVALID_SOCKET);
    TBaseServerRequestData(std::string_view qs, SOCKET s = INVALID_SOCKET);

    void SetHost(const std::string& host, ui16 port) {
        Host_ = host;
        Port_ = ToString(port);
    }

    const std::string& ServerName() const {
        return Host_;
    }

    NAddr::IRemoteAddrPtr ServerAddress() const {
        return NAddr::GetSockAddr(Socket_);
    }

    const std::string& ServerPort() const {
        return Port_;
    }

    std::string_view ScriptName() const {
        return Path_;
    }

    std::string_view Query() const {
        return Query_;
    }

    std::string_view OrigQuery() const {
        return OrigQuery_;
    }

    void AppendQueryString(std::string_view str);
    std::string_view RemoteAddr() const;
    void SetRemoteAddr(std::string_view addr);
    // Returns nullptr when the header does not exist
    const std::string* HeaderIn(std::string_view key) const;
    // Throws on missing header
    std::string_view HeaderInOrEmpty(std::string_view key) const;

    const THttpHeadersContainer& HeadersIn() const {
        return HeadersIn_;
    }

    inline size_t HeadersCount() const noexcept {
        return HeadersIn_.size();
    }

    std::string HeaderByIndex(size_t n) const noexcept;
    std::string_view Environment(std::string_view key) const;

    void Clear();

    void SetSocket(SOCKET s) noexcept {
        Socket_ = s;
    }

    ui64 RequestBeginTime() const noexcept {
        return BeginTime_;
    }

    void SetPath(std::string path);
    const std::string& GetCurPage() const;
    bool Parse(std::string_view req);
    void AddHeader(const std::string& name, const std::string& value);

private:
    mutable TMaybe<std::string> Addr_;
    std::string Host_;
    std::string Port_;
    std::string Path_;
    std::string_view Query_;
    std::string_view OrigQuery_;
    THttpHeadersContainer HeadersIn_;
    SOCKET Socket_;
    ui64 BeginTime_;
    mutable std::string CurPage_;
    std::vector<char> ParseBuf_;
    std::string ModifiedQueryString_;
};

class TServerRequestData: public TBaseServerRequestData {
public:
    TServerRequestData(SOCKET s = INVALID_SOCKET)
        : TBaseServerRequestData(s)
    {
    }
    TServerRequestData(std::string_view qs, SOCKET s = INVALID_SOCKET)
        : TBaseServerRequestData(qs, s)
    {
        Scan();
    }

    void Scan() {
        CgiParam.Scan(Query());
    }

public:
    TCgiParameters CgiParam;
};
