#pragma once

#include "iovec.h"

#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/system/platform.h>


namespace NUtils::NIOVector {

    using TPart = IOutputStream::TPart;

    using TIOVectorAdaptorTraits =
#if !defined(_win_)
            TSpanAdapterTraits<iovec, &iovec::iov_base, &iovec::iov_len, IOV_MAX>
                ::With<TPart, &TPart::buf, &TPart::len,
                    offsetof(TPart, buf) == offsetof(iovec, iov_base)
                        && offsetof(TPart, len) == offsetof(iovec, iov_len)>;
#else // defined(_win_)
            TSpanAdapterTraits<WSABUF, &WSABUF::buf, &WSABUF::len, std::numeric_limits<DWORD>::max()>
                ::With<TPart, &TPart::buf, &TPart::len,
                    offsetof(TPart, buf) == offsetof(WSABUF, buf)
                        && offsetof(TPart, len) == offsetof(WSABUF, len)>;
#endif // defined(_win_)

    struct TReadVHandler {
        using IOVector = TIOVectorAdaptorTraits::LowLevel::Type;

        ssize_t operator()(SOCKET socket, const IOVector* to, std::size_t count) const noexcept {
#if !defined(_win_)
            return readv(socket, to, static_cast<int>(count));
#else // defined(_win_)
            DWORD n_bytes_recv;
            DWORD flags = 0;
            const int res = WSARecv(socket,
                                    to, static_cast<DWORD>(count),
                                    &n_bytes_recv, &flags, nullptr, nullptr);
            if (res == SOCKET_ERROR) {
                errno = EIO;
                return -1;
            }
            return n_bytes_recv;
#endif // defined(_win_)
        }
    };

    struct TWriteVHandler {
        using IOVector = TIOVectorAdaptorTraits::LowLevel::Type;

        ssize_t operator()(SOCKET socket, const IOVector* from, std::size_t count) const noexcept {
#if !defined(_win_)
            return writev(socket, from, static_cast<int>(count));
#else // defined(_win_)
            DWORD n_bytes_recv;
            const int res = WSASend(static_cast<SOCKET>(socket),
                                    from, static_cast<DWORD>(count),
                                    &n_bytes_recv, 0, nullptr, nullptr);
            if (res == SOCKET_ERROR) {
                errno = EIO;
                return -1;
            }
            return static_cast<ssize_t>(n_bytes_recv);
#endif // defined(_win_)
        }
    };

    template <typename THandler>
    using TContIOVectorWith = TContIOVector<TIOVectorAdaptorTraits, THandler>;

    using TContIOVectorReader = TContIOVectorWith<TReadVHandler>;
    using TContIOVectorWriter = TContIOVectorWith<TWriteVHandler>;

} // namespace NUtils::NIOVector
