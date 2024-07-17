#include "fast.h"
#include <ydb-cpp-sdk/util/random/random.h>
#include "entropy.h"
#include "mersenne.h"
#include "shuffle.h"
#include "init_atfork.h"

#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/stream/mem.h>
#include <src/util/stream/zlib.h>
#include <src/util/stream/buffer.h>
#include <src/util/stream/buffered.h>

#include <src/util/system/fs.h>
#include <src/util/system/info.h>
#include <ydb-cpp-sdk/util/system/spinlock.h>
#include <src/util/system/thread.h>
#include <src/util/system/execpath.h>
#include <src/util/system/datetime.h>
#include <src/util/system/hostname.h>
#include <src/util/system/getpid.h>
#include <src/util/system/mem_info.h>
#include <src/util/system/rusage.h>
#include <src/util/system/cpu_id.h>
#include <ydb-cpp-sdk/util/system/unaligned_mem.h>

#include <src/util/generic/buffer.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>

#include <src/util/digest/murmur.h>
#include <src/util/digest/city.h>

#include <ydb-cpp-sdk/util/ysaveload.h>

#include <memory>

namespace {
    inline void Permute(char* buf, size_t len, ui32 seed) noexcept {
        Shuffle(buf, buf + len, TReallyFastRng32(seed));
    }

    struct THostEntropy: public TBuffer {
        inline THostEntropy() {
            {
                TBufferOutput buf(*this);
                TZLibCompress out(&buf);

                Save(&out, GetPID());
                Save(&out, GetCycleCount());
                Save(&out, MicroSeconds());
                Save(&out, TThread::CurrentThreadId());
                Save(&out, NSystemInfo::CachedNumberOfCpus());
                Save(&out, NSystemInfo::TotalMemorySize());
                Save(&out, HostName());

                try {
                    Save(&out, GetExecPath());
                } catch (...) {
                    //workaround - sometimes fails on FreeBSD
                }

                Save(&out, (size_t)Data());
                Save(&out, (size_t)&buf);

                {
                    double la[3];

                    NSystemInfo::LoadAverage(la, Y_ARRAY_SIZE(la));

                    out.Write(la, sizeof(la));
                }

                {
                    auto mi = NMemInfo::GetMemInfo();

                    out.Write(&mi, sizeof(mi));
                }

                {
                    auto ru = TRusage::Get();

                    out.Write(&ru, sizeof(ru));
                }

                {
                    ui32 store[12];

                    out << std::string_view(CpuBrand(store));
                }

                out << NFs::CurrentWorkingDirectory();

                out.Finish();
            }

            {
                TMemoryOutput out(Data(), Size());

                //replace zlib header with hash
                Save(&out, CityHash64(Data(), Size()));
            }

            Permute(Data(), Size(), MurmurHash<ui32>(Data(), Size()));
        }
    };

    //not thread-safe
    class TMersenneInput: public IInputStream {
        using TKey = ui64;
        using TRnd = TMersenne<TKey>;

    public:
        inline explicit TMersenneInput(const TBuffer& rnd)
            : Rnd_((const TKey*)rnd.Data(), rnd.Size() / sizeof(TKey))
        {
        }

        ~TMersenneInput() override = default;

        size_t DoRead(void* buf, size_t len) override {
            size_t toRead = len;

            while (toRead) {
                const TKey next = Rnd_.GenRand();
                const size_t toCopy = Min(toRead, sizeof(next));

                memcpy(buf, &next, toCopy);

                buf = (char*)buf + toCopy;
                toRead -= toCopy;
            }

            return len;
        }

    private:
        TRnd Rnd_;
    };

    class TEntropyPoolStream: public IInputStream {
    public:
        inline explicit TEntropyPoolStream(const TBuffer& buffer)
            : Mi_(buffer)
            , Bi_(&Mi_, 8192)
        {
        }

        size_t DoRead(void* buf, size_t len) override {
            auto guard = Guard(Mutex_);

            return Bi_.Read(buf, len);
        }

    private:
        TAdaptiveLock Mutex_;
        TMersenneInput Mi_;
        TBufferedInput Bi_;
    };

    struct TSeedStream: public IInputStream {
        size_t DoRead(void* inbuf, size_t len) override {
            char* buf = (char*)inbuf;

#define DO_STEP(type)                                    \
    while (len >= sizeof(type)) {                        \
        WriteUnaligned<type>(buf, RandomNumber<type>()); \
        buf += sizeof(type);                             \
        len -= sizeof(type);                             \
    }

            DO_STEP(ui64);
            DO_STEP(ui32);
            DO_STEP(ui16);
            DO_STEP(ui8);

#undef DO_STEP

            return buf - (char*)inbuf;
        }
    };

    struct TDefaultTraits {
        std::unique_ptr<TEntropyPoolStream> EP;
        TSeedStream SS;

        inline TDefaultTraits() {
            Reset();
        }

        inline IInputStream& EntropyPool() noexcept {
            return *EP;
        }

        inline IInputStream& Seed() noexcept {
            return SS;
        }

        inline void Reset() noexcept {
            EP = std::make_unique<TEntropyPoolStream>(THostEntropy());
        }

        static inline TDefaultTraits& Instance() {
            auto res = SingletonWithPriority<TDefaultTraits, 0>();

            RNGInitAtForkHandlers();

            return *res;
        }
    };

    using TRandomTraits = TDefaultTraits;
}

IInputStream& EntropyPool() {
    return TRandomTraits::Instance().EntropyPool();
}

IInputStream& Seed() {
    return TRandomTraits::Instance().Seed();
}

void ResetEntropyPool() {
    TRandomTraits::Instance().Reset();
}
