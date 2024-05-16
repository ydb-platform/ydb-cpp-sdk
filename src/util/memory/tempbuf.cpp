#include "addstorage.h"

#include <src/util/generic/intrlist.h>
#include <src/util/thread/singleton.h>

#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
#include <ydb-cpp-sdk/util/memory/tempbuf.h>
#include <ydb-cpp-sdk/util/system/yassert.h>
#include <ydb-cpp-sdk/util/system/defaults.h>

#include <utility>


class TTempBuf::TImpl: public TRefCounted<TImpl, TSimpleCounter, TImpl> {
public:
    TImpl(void* data, std::size_t size) noexcept
        : Data_(data)
        , Size_(size)
        , Offset_(0)
    {
    }

    /*
     * We do not really need 'virtual' here
     * but for compiler happiness...
     */
    virtual ~TImpl() = default;

    void* Data() noexcept {
        return Data_;
    }

    const void* Data() const noexcept {
        return Data_;
    }

    std::size_t Size() const noexcept {
        return Size_;
    }

    std::size_t Filled() const noexcept {
        return Offset_;
    }

    void Reset() noexcept {
        Offset_ = 0;
    }

    std::size_t Left() const noexcept {
        return Size() - Filled();
    }

    void SetPos(std::size_t off) {
        Y_ASSERT(off <= Size());
        Offset_ = off;
    }

    void Proceed(std::size_t off) {
        Y_ASSERT(off <= Left());

        Offset_ += off;
    }

    static void Destroy(TImpl* This) noexcept {
        This->Dispose();
    }

protected:
    virtual void Dispose() noexcept = 0;

private:
    void* Data_;
    std::size_t Size_;
    std::size_t Offset_;
};

namespace {
    class TTempBufManager;

    class TAllocedBuf: public TTempBuf::TImpl, public TAdditionalStorage<TAllocedBuf> {
    public:
        TAllocedBuf()
            : TImpl(AdditionalData(), AdditionalDataLength())
        {
        }

        ~TAllocedBuf() override = default;

    private:
        void Dispose() noexcept override {
            delete this;
        }
    };

    class TPerThreadedBuf: public TTempBuf::TImpl, public TIntrusiveSListItem<TPerThreadedBuf> {
        friend class TTempBufManager;

    public:
        inline TPerThreadedBuf(TTempBufManager* manager) noexcept
            : TImpl(Data_, sizeof(Data_))
            , Manager_(manager)
        {
        }

        ~TPerThreadedBuf() override = default;

    private:
        void Dispose() noexcept override;

    private:
        alignas(std::max_align_t) char Data_[TTempBuf::TmpBufLen];
        TTempBufManager* Manager_;
    };

    class TTempBufManager {
        struct TDelete {
            void operator()(TPerThreadedBuf* p) noexcept {
                delete p;
            }
        };

    public:
        TTempBufManager() noexcept = default;

        ~TTempBufManager() {
            TDelete deleter;

            Unused_.ForEach(deleter);
        }

        TPerThreadedBuf* Acquire() {
            if (!Unused_.Empty()) {
                return Unused_.PopFront();
            }

            return new TPerThreadedBuf(this);
        }

        void Return(TPerThreadedBuf* buf) noexcept {
            buf->Reset();
            Unused_.PushFront(buf);
        }

    private:
        TIntrusiveSList<TPerThreadedBuf> Unused_;
    };
}

static inline TTempBufManager* TempBufManager() {
    return FastTlsSingletonWithPriority<TTempBufManager, 2>();
}

static inline TTempBuf::TImpl* AcquireSmallBuffer([[maybe_unused]] std::size_t size) {
#if defined(_asan_enabled_)
    return new (size) TAllocedBuf();
#else
    return TempBufManager()->Acquire();
#endif
}

void TPerThreadedBuf::Dispose() noexcept {
    if (Manager_ == TempBufManager()) {
        Manager_->Return(this);
    } else {
        delete this;
    }
}

TTempBuf::TTempBuf()
    : Impl_(AcquireSmallBuffer(TTempBuf::TmpBufLen))
{
}

/*
 * all magick is here:
 * if len <= TTempBuf::TmpBufLen. then we get prealloced per threaded buffer
 * else allocate one in heap
 */
static inline TTempBuf::TImpl* ConstructImpl(std::size_t len) {
    if (len <= TTempBuf::TmpBufLen) {
        return AcquireSmallBuffer(len);
    }

    return new (len) TAllocedBuf();
}

TTempBuf::TTempBuf(std::size_t len)
    : Impl_(ConstructImpl(len))
{
}

TTempBuf::TTempBuf(const TTempBuf&) noexcept = default;

TTempBuf::TTempBuf(TTempBuf&& b) noexcept
    : Impl_(std::move(b.Impl_))
{
}

TTempBuf::~TTempBuf() = default;

TTempBuf& TTempBuf::operator=(const TTempBuf& b) noexcept {
    if (this != &b) {
        Impl_ = b.Impl_;
    }

    return *this;
}

TTempBuf& TTempBuf::operator=(TTempBuf&& b) noexcept {
    if (this != &b) {
        Impl_ = std::move(b.Impl_);
    }

    return *this;
}

char* TTempBuf::Data() noexcept {
    return static_cast<char*>(Impl_->Data());
}

const char* TTempBuf::Data() const noexcept {
    return static_cast<const char*>(Impl_->Data());
}

std::size_t TTempBuf::Size() const noexcept {
    return Impl_->Size();
}

char* TTempBuf::Current() noexcept {
    return Data() + Filled();
}

const char* TTempBuf::Current() const noexcept {
    return Data() + Filled();
}

std::size_t TTempBuf::Filled() const noexcept {
    return Impl_->Filled();
}

std::size_t TTempBuf::Left() const noexcept {
    return Impl_->Left();
}

void TTempBuf::Reset() noexcept {
    Impl_->Reset();
}

void TTempBuf::SetPos(std::size_t off) {
    Impl_->SetPos(off);
}

char* TTempBuf::Proceed(std::size_t off) {
    char* ptr = Current();
    Impl_->Proceed(off);
    return ptr;
}

void TTempBuf::Append(const void* data, std::size_t len) {
    if (len > Left()) {
        ythrow yexception() << "temp buf exhausted(" << Left() << ", " << len << ")";
    }

    memcpy(Current(), data, len);
    Proceed(len);
}

bool TTempBuf::IsNull() const noexcept {
    return !Impl_;
}

#if 0
    #include <src/util/datetime/cputimer.h>

    #define LEN (1024)

void* allocaFunc() {
    return alloca(LEN);
}

int main() {
    const std::size_t num = 10000000;
    std::size_t tmp = 0;

    {
        CTimer t("alloca");

        for (std::size_t i = 0; i < num; ++i) {
            tmp += (std::size_t)allocaFunc();
        }
    }

    {
        CTimer t("log buffer");

        for (std::size_t i = 0; i < num; ++i) {
            TTempBuf buf(LEN);

            tmp += (std::size_t)buf.Data();
        }
    }

    {
        CTimer t("malloc");

        for (std::size_t i = 0; i < num; ++i) {
            void* ptr = malloc(LEN);

            tmp += (std::size_t)ptr;

            free(ptr);
        }
    }

    return 0;
}
#endif
