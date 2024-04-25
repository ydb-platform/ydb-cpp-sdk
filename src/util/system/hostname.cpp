<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> origin/main
#include <ydb-cpp-sdk/library/string_utils/misc/misc.h>

#include <ydb-cpp-sdk/util/memory/tempbuf.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
#include <ydb-cpp-sdk/util/network/ip.h>
<<<<<<< HEAD
=======
#include <src/library/string_utils/misc/misc.h>

#include <src/util/memory/tempbuf.h>
#include <src/util/generic/singleton.h>
#include <src/util/generic/yexception.h>
#include <src/util/network/ip.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main

#if defined(_unix_)
    #include <unistd.h>
    #include <ifaddrs.h>
    #include <netdb.h>
#endif

#if defined(_win_)
    #include <WinSock2.h>
#endif

#include <ydb-cpp-sdk/util/system/defaults.h>
#include <ydb-cpp-sdk/util/system/yassert.h>
#include "hostname.h"

namespace {
    struct THostNameHolder {
        inline THostNameHolder() {
            TTempBuf hostNameBuf;

            if (gethostname(hostNameBuf.Data(), hostNameBuf.Size() - 1)) {
                ythrow TSystemError() << "can not get host name";
            }

            HostName = hostNameBuf.Data();
        }

        std::string HostName;
    };

    struct TFQDNHostNameHolder {
        inline TFQDNHostNameHolder() {
            char buf[1024];

            memset(buf, 0, sizeof(buf));
            int res = gethostname(buf, sizeof(buf) - 1);
            if (res) {
                ythrow TSystemError() << "can not get hostname";
            }

#ifdef _darwin_
            // On Darwin gethostname returns fqdn, see hostname.c in shell_cmds:
            // https://github.com/apple-oss-distributions/shell_cmds/blob/main/hostname/hostname.c
            // There are macs in the wild that don't have fqdn hostnames, but
            // which have search domains in their resolv.conf, so any attempt to
            // resolve AI_CANONNAME for the short hostname will result in the
            // EAI_NONAME error.
            // It seems using gethostname is enough to emulate the result of
            // `hostname -f`, which still works on those macs.
            FQDNHostName = buf;
#else
            // On Linux `hostname -f` calls getaddrinfo with AI_CANONNAME flag
            // to find the fqdn and will fail on any error.
            // Hosts often have a short hostname and fqdn is resolved over dns.
            // It is also very common to have a short hostname alias in
            // /etc/hosts, which works as a fallback (e.g. no fqdn in search
            // domains, otherwise `hostname -f` fails with an error and it is
            // obvious the host is not configured correctly).
            // Note that getaddrinfo may sometimes return EAI_NONAME even when
            // host actually has fqdn, but dns is temporary unavailable, so we
            // cannot ignore any errors on Linux.
            struct addrinfo hints;
            struct addrinfo* ais{nullptr};

            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_flags = AI_CANONNAME;
            res = getaddrinfo(buf, nullptr, &hints, &ais);
            if (res) {
                ythrow TSystemError() << "can not get FQDN (return code is " << res << ", hostname is \"" << buf << "\")";
            }
            FQDNHostName = ais->ai_canonname;
            NUtils::ToLower(FQDNHostName);
            freeaddrinfo(ais);
#endif
        }

        std::string FQDNHostName;
    };
}

const std::string& HostName() {
    return (Singleton<THostNameHolder>())->HostName;
}

const char* GetHostName() {
    return HostName().data();
}

const std::string& FQDNHostName() {
    return (Singleton<TFQDNHostNameHolder>())->FQDNHostName;
}

const char* GetFQDNHostName() {
    return FQDNHostName().data();
}

bool IsFQDN(const std::string& name) {
    std::string absName = name;
    if (!absName.ends_with('.')) {
        absName.append(".");
    }

    try {
        // ResolveHost() can't be used since it is ipv4-only, port is not important
        TNetworkAddress addr(absName, 0);
    } catch (const TNetworkResolutionError&) {
        return false;
    }
    return true;
}
