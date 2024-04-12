#pragma once

#include <src/library/digest/lower_case/hash_ops.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/defaults.h>
#include <ydb-cpp-sdk/library/cgiparam/cgiparam.h>
#include <ydb-cpp-sdk/util/network/address.h>
#include <ydb-cpp-sdk/util/network/socket.h>
#include <src/util/generic/hash.h>
#include <ydb-cpp-sdk/util/system/yassert.h>
#include <ydb-cpp-sdk/util/datetime/base.h>
=======
#include <src/util/system/defaults.h>
#include <src/library/cgiparam/cgiparam.h>
#include <src/util/network/address.h>
#include <src/util/network/socket.h>
#include <src/util/generic/hash.h>
#include <src/util/system/yassert.h>
#include <src/util/datetime/base.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

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
    mutable std::optional<std::string> Addr_;
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
