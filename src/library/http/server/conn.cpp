#include <ydb-cpp-sdk/library/http/server/conn.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/network/socket.h>
=======
#include <src/util/network/socket.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/network/socket.h>
>>>>>>> origin/main
#include <src/util/stream/buffered.h>

class THttpServerConn::TImpl {
public:
    inline TImpl(const TSocket& s, size_t outputBufferSize)
        : S_(s)
        , SI_(S_)
        , SO_(S_)
        , BO_(&SO_, outputBufferSize)
        , HI_(&SI_)
        , HO_(&BO_, &HI_)
    {
    }

    inline ~TImpl() {
    }

    inline THttpInput* Input() noexcept {
        return &HI_;
    }

    inline THttpOutput* Output() noexcept {
        return &HO_;
    }

    inline void Reset() {
        if (S_ != INVALID_SOCKET) {
            // send RST packet to client
            S_.SetLinger(true, 0);
            S_.Close();
        }
    }

private:
    TSocket S_;
    TSocketInput SI_;
    TSocketOutput SO_;
    TBufferedOutput BO_;
    THttpInput HI_;
    THttpOutput HO_;
};

THttpServerConn::THttpServerConn(const TSocket& s)
    : THttpServerConn(s, s.MaximumTransferUnit())
{
}

THttpServerConn::THttpServerConn(const TSocket& s, size_t outputBufferSize)
    : Impl_(new TImpl(s, outputBufferSize))
{
}

THttpServerConn::~THttpServerConn() {
}

THttpInput* THttpServerConn::Input() noexcept {
    return Impl_->Input();
}

THttpOutput* THttpServerConn::Output() noexcept {
    return Impl_->Output();
}

void THttpServerConn::Reset() {
    return Impl_->Reset();
}
