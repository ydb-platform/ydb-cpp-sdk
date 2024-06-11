#include "tls.h"

#include <src/util/generic/intrlist.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>

#include <atomic>

#if defined(_unix_)
    #include <pthread.h>
#endif

#include <unordered_map>

using namespace NTls;

namespace {
    static inline size_t AcquireKey() {
        static std::atomic<size_t> cur;

        return cur++;
    }

    class TGenericTlsBase {
    public:
        using TSmallKey = size_t;

        class TPerThreadStorage {
        public:
            struct TKey: public TNonCopyable {
                inline TKey(TDtor dtor)
                    : Key(AcquireKey())
                    , Dtor(dtor)
                {
                }

                TSmallKey Key;
                TDtor Dtor;
            };

            class TStoredValue: public TIntrusiveListItem<TStoredValue> {
            public:
                inline TStoredValue(const TKey* key)
                    : Data_(nullptr)
                    , Dtor_(key->Dtor)
                {
                }

                inline ~TStoredValue() {
                    if (Dtor_ && Data_) {
                        Dtor_(Data_);
                    }
                }

                inline void Set(void* ptr) noexcept {
                    Data_ = ptr;
                }

                inline void* Get() const noexcept {
                    return Data_;
                }

            private:
                void* Data_;
                TDtor Dtor_;
            };

            inline TStoredValue* Value(const TKey* key) {
                TStoredValue*& ret = *ValuePtr((size_t)key->Key);

                if (!ret) {
                    THolder<TStoredValue> sv(new TStoredValue(key));

                    Storage_.PushFront(sv.Get());
                    ret = sv.Release();
                }

                return ret;
            }

            inline TStoredValue** ValuePtr(size_t idx) {
                // do not grow vector too much
                if (idx < 10000) {
                    if (idx >= Values_.size()) {
                        Values_.resize(idx + 1);
                    }

                    return &Values_[idx];
                }

                return &FarValues_[idx];
            }

        private:
            std::vector<TStoredValue*> Values_;
            std::unordered_map<size_t, TStoredValue*> FarValues_;
            TIntrusiveListWithAutoDelete<TStoredValue, TDelete> Storage_;
        };

        inline TPerThreadStorage* MyStorage() {
#if defined(Y_HAVE_FAST_POD_TLS)
            Y_POD_STATIC_THREAD(TPerThreadStorage*)
            my(nullptr);

            if (!my) {
                my = MyStorageSlow();
            }

            return my;
#else
            return MyStorageSlow();
#endif
        }

        virtual TPerThreadStorage* MyStorageSlow() = 0;

        virtual ~TGenericTlsBase() = default;
    };
}

#if defined(_unix_)
namespace {
    class TMasterTls: public TGenericTlsBase {
    public:
        inline TMasterTls() {
            Y_ABORT_UNLESS(!pthread_key_create(&Key_, Dtor), "pthread_key_create failed");
        }

        inline ~TMasterTls() override {
            //explicitly call dtor for main thread
            Dtor(pthread_getspecific(Key_));

            Y_ABORT_UNLESS(!pthread_key_delete(Key_), "pthread_key_delete failed");
        }

        static inline TMasterTls* Instance() {
            return SingletonWithPriority<TMasterTls, 1>();
        }

    private:
        TPerThreadStorage* MyStorageSlow() override {
            void* ret = pthread_getspecific(Key_);

            if (!ret) {
                ret = new TPerThreadStorage();

                Y_ABORT_UNLESS(!pthread_setspecific(Key_, ret), "pthread_setspecific failed");
            }

            return (TPerThreadStorage*)ret;
        }

        static void Dtor(void* ptr) {
            delete (TPerThreadStorage*)ptr;
        }

    private:
        pthread_key_t Key_;
    };

    using TKeyDescriptor = TMasterTls::TPerThreadStorage::TKey;
}

class TKey::TImpl: public TKeyDescriptor {
public:
    inline TImpl(TDtor dtor)
        : TKeyDescriptor(dtor)
    {
    }

    inline void* Get() const {
        return TMasterTls::Instance()->MyStorage()->Value(this)->Get();
    }

    inline void Set(void* val) const {
        TMasterTls::Instance()->MyStorage()->Value(this)->Set(val);
    }

    static inline void Cleanup() {
    }
};
#else
namespace {
    class TGenericTls: public TGenericTlsBase {
    public:
        virtual TPerThreadStorage* MyStorageSlow() {
            std::lock_guard guard(Lock_);

            {
                TPTSRef& ret = Datas_[TThread::CurrentThreadId()];

                if (!ret) {
                    ret.Reset(new TPerThreadStorage());
                }

                return ret.Get();
            }
        }

        inline void Cleanup() noexcept {
            std::lock_guard guard(Lock_);
            Datas_.erase(TThread::CurrentThreadId());
        }

        static inline TGenericTls* Instance() {
            return SingletonWithPriority<TGenericTls, 1>();
        }

    private:
        using TPTSRef = THolder<TPerThreadStorage>;
        std::mutex Lock_;
        std::unordered_map<TThread::TId, TPTSRef> Datas_;
    };
}

class TKey::TImpl {
public:
    inline TImpl(TDtor dtor)
        : Key_(dtor)
    {
    }

    inline void* Get() {
        return TGenericTls::Instance()->MyStorage()->Value(&Key_)->Get();
    }

    inline void Set(void* ptr) {
        TGenericTls::Instance()->MyStorage()->Value(&Key_)->Set(ptr);
    }

    static inline void Cleanup() {
        TGenericTls::Instance()->Cleanup();
    }

private:
    TGenericTls::TPerThreadStorage::TKey Key_;
};
#endif

TKey::TKey(TDtor dtor)
    : Impl_(new TImpl(dtor))
{
}

TKey::TKey(TKey&&) noexcept = default;

TKey::~TKey() = default;

void* TKey::Get() const {
    return Impl_->Get();
}

void TKey::Set(void* ptr) const {
    Impl_->Set(ptr);
}

void TKey::Cleanup() noexcept {
    TImpl::Cleanup();
}
