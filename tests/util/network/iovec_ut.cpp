#include "vectored_io.h"

#include <src/library/testing/gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <iterator>
#include <limits>
#include <random>
#include <string>
#include <sstream>
#include <vector>

using namespace NUtils::NIOVector;

template <typename T>
struct TSpan1 {
    T* buf;
    std::size_t len;
};

template <typename T>
struct TSpan2 {
    std::size_t len;
    T* buf;
};

template <typename T>
using TCompatSpan =
#if !defined(_win_)
        TSpan1<T>;
#else // defined(_win_)
        TSpan2<T>;
#endif // defined(_win_)

template <typename T>
using TIncompatSpan =
#if !defined(_win_)
        TSpan2<T>;
#else // defined(_win_)
        TSpan1<T>;
#endif // defined(_win_)

template <typename T, std::size_t Max>
using TCompatTraits =
#if !defined(_win_)
        TSpanAdapterTraits<iovec, &iovec::iov_base, &iovec::iov_len, Max>
            ::template With<TCompatSpan<T>, &TCompatSpan<T>::buf, &TCompatSpan<T>::len,
                            offsetof(TCompatSpan<T>, buf) == offsetof(iovec, iov_base)
                                && offsetof(TCompatSpan<T>, len) == offsetof(iovec, iov_len)>;
#else // defined(_win_)
        TSpanAdapterTraits<WSABUF, &WSABUF::buf, &WSABUF::len, Max>
            ::template With<TCompatSpan<T>, &TCompatSpan<T>::buf, &TCompatSpan<T>::len,
                            offsetof(TCompatSpan<T>, buf) == offsetof(WSABUF, buf)
                                && offsetof(TCompatSpan<T>, len) == offsetof(WSABUF, len)>;
#endif // defined(_win_)

template <typename T, std::size_t Max>
using TIncompatTraits =
#if !defined(_win_)
        TSpanAdapterTraits<iovec, &iovec::iov_base, &iovec::iov_len, Max>
            ::template With<TIncompatSpan<T>, &TIncompatSpan<T>::buf, &TIncompatSpan<T>::len,
                            offsetof(TIncompatSpan<T>, buf) == offsetof(iovec, iov_base)
                                && offsetof(TIncompatSpan<T>, len) == offsetof(iovec, iov_len)>;
#else // defined(_win_)
        TSpanAdapterTraits<WSABUF, &WSABUF::buf, &WSABUF::len, Max>
            ::template With<TIncompatSpan<T>, &TIncompatSpan<T>::buf, &TIncompatSpan<T>::len,
                            offsetof(TIncompatSpan<T>, buf) == offsetof(WSABUF, buf)
                                && offsetof(TIncompatSpan<T>, len) == offsetof(WSABUF, len)>;
#endif // defined(_win_)

template <typename T>
struct TStdLikeSpan1 {
// as if private:
    T* buf;
    std::size_t len;

    TStdLikeSpan1() = default;
public:
    TStdLikeSpan1(T* buf, std::size_t size) noexcept : buf(buf), len(size) {}
    T* Data() const noexcept { return buf; }
    std::size_t Size() const noexcept { return len; }
};

template <typename T>
struct TStdLikeSpan2 {
// as if private:
    std::size_t len;
    T* buf;

    TStdLikeSpan2() = default;
public:
    TStdLikeSpan2(T* buf, std::size_t size) noexcept : len(size), buf(buf) {}
    T* Data() const noexcept { return buf; }
    std::size_t Size() const noexcept { return len; }
};

template <typename T>
using TCompatStdLikeSpan =
#if !defined(_win_)
        TStdLikeSpan1<T>;
#else // defined(_win_)
        TStdLikeSpan2<T>;
#endif // defined(_win_)

template <typename T>
using TIncompatStdLikeSpan =
#if !defined(_win_)
        TStdLikeSpan2<T>;
#else // defined(_win_)
        TStdLikeSpan1<T>;
#endif // defined(_win_)

template <typename T, std::size_t Max>
using TCompatStdLikeSpanTrains =
#if !defined(_win_)
        TSpanAdapterTraits<iovec, &iovec::iov_base, &iovec::iov_len, Max>
            ::template With<TCompatStdLikeSpan<T>,
                            &TCompatStdLikeSpan<T>::Data, &TCompatStdLikeSpan<T>::Size>;
#else // defined(_win_)
        TSpanAdapterTraits<iovec, &WSABUF::buf, &WSABUF::len, Max>
            ::template With<TCompatStdLikeSpan<T>,
                            &TCompatStdLikeSpan<T>::Data, &TCompatStdLikeSpan<T>::Size>;
#endif // defined(_win_)
        

template <typename T, std::size_t Max>
using TIncompatStdLikeSpanTrains =
#if !defined(_win_)
        TSpanAdapterTraits<iovec, &iovec::iov_base, &iovec::iov_len, Max>
            ::template With<TIncompatStdLikeSpan<T>,
                            &TIncompatStdLikeSpan<T>::Data, &TIncompatStdLikeSpan<T>::Size>;
#else // defined(_win_)
        TSpanAdapterTraits<iovec, &WSABUF::buf, &WSABUF::len, Max>
            ::template With<TIncompatStdLikeSpan<T>,
                            &TIncompatStdLikeSpan<T>::Data, &TIncompatStdLikeSpan<T>::Size>;
#endif // defined(_win_)

template <typename TPtr>
char* ToCharPtr(TPtr ptr) noexcept {
    return const_cast<char*>(reinterpret_cast<const char*>(ptr));
}

template <typename TTraits>
struct TMockReaderHandler {
    static constexpr auto max_threshold = std::numeric_limits<std::size_t>::max();

    static inline auto threshold = max_threshold;
    static inline std::istringstream input;

    ssize_t operator()(int, const TTraits::LowLevel::Type* iov, std::size_t count) const noexcept {
        using L = typename TTraits::LowLevel;

        const auto max_bytes = threshold;

        std::size_t bytes = 0;
        for (auto iter = iov, end = iov + count; iter != end; bytes += L::Length(*iter), ++iter) {
            const auto new_len = bytes + L::Length(*iter);
            if (new_len > max_bytes) {
                input.read(ToCharPtr(L::PtrBase(*iter)),
                           static_cast<std::streamsize>(max_bytes - bytes));
                bytes += max_bytes - bytes;
                return static_cast<ssize_t>(bytes);
            }
            input.read(ToCharPtr(L::PtrBase(*iter)),
                       static_cast<std::streamsize>(L::Length(*iter)));
        }
        return static_cast<ssize_t>(bytes);
    }
};

template <typename TTraits>
struct TMockWriterHandler {
    static constexpr auto max_threshold = std::numeric_limits<std::size_t>::max();

    static inline auto threshold = max_threshold;
    static inline std::ostringstream output;

    ssize_t operator()(int, const TTraits::LowLevel::Type* iov, std::size_t count) const noexcept {
        using L = typename TTraits::LowLevel;

        const auto max_bytes = threshold;

        std::size_t bytes = 0;
        for (auto iter = iov, end = iov + count; iter != end; bytes += L::Length(*iter), ++iter) {
            const auto new_len = bytes + L::Length(*iter);
            if (new_len > max_bytes) {
                output.write(ToCharPtr(L::PtrBase(*iter)),
                             static_cast<std::streamsize>(max_bytes - bytes));
                bytes += max_bytes - bytes;
                return static_cast<ssize_t>(bytes);
            }
            output.write(ToCharPtr(L::PtrBase(*iter)),
                         static_cast<std::streamsize>(L::Length(*iter)));
        }
        return static_cast<ssize_t>(bytes);
    }
};

class Generator {
    static inline std::random_device rd;
    static inline auto gen = std::mt19937(rd());
    static inline auto char_distrib = std::uniform_int_distribution(
            static_cast<int>('a'), static_cast<int>('z'));
    static inline auto size_distrib = std::uniform_int_distribution<std::size_t>(10, 100);
public:
    static char RandomChar() { return static_cast<char>(char_distrib(gen)); }
    static std::string RandomString(std::size_t n) {
        std::string str;
        str.reserve(n);
        std::generate_n(std::back_inserter(str), n, RandomChar);
        return str;
    }
    static std::size_t RandomSize() { return size_distrib(gen); }
};

template <typename Chunk, typename TPtrBase, std::size_t Max, std::size_t Factor>
std::vector<Chunk> SplitBufferToChunks(char* begin, const char* end) {
    const auto buffer_len = static_cast<std::size_t>(end - begin);
    
    std::vector<Chunk> chunks;
    chunks.reserve(Factor * Max);

    std::size_t lens = 0;
    for (std::size_t chunk_len = 0; lens < buffer_len; lens += chunk_len) {
        chunk_len = Generator::RandomSize() % 10 + 1;
        if (lens + chunk_len > buffer_len) {
            chunk_len = buffer_len - lens;
        }
        Chunk chunk;
        chunk.buf = reinterpret_cast<TPtrBase>(begin + lens);
        chunk.len = chunk_len;
        chunks.push_back(chunk);
    }
    return chunks;
}

template <typename Chunk>
std::vector<std::size_t> PartialSums(const std::vector<Chunk>& chunks) noexcept {
    std::vector<std::size_t> partial_sums;
    partial_sums.reserve(chunks.size() + 1);
    std::size_t acc = 0;
    partial_sums.push_back(acc);
    for (auto iter = chunks.cbegin(), end = chunks.cend(); iter != end; ++iter) {
        partial_sums.push_back(partial_sums.back() + iter->len);
    }
    return partial_sums;
}

template <typename TTraits1, typename TTraits2 = TTraits1>
void TestIOVectorWithMockHandler() {
    using H = typename TTraits1::HighLevel::Type;
    using HPtr = typename TTraits1::HighLevel::TPtrBase;

    using Reader = TMockReaderHandler<TTraits1>;
    using Writer = TMockWriterHandler<TTraits2>;
    
    constexpr int dummy_fd = 1;

    constexpr std::size_t Max = TTraits1::LowLevel::MaxCount;
    constexpr std::size_t Factor = 2;
    constexpr std::size_t StrLen = Factor * 10 * Max;
    constexpr std::size_t Threshold = StrLen / 13;
    constexpr std::size_t MaxThreshold = std::numeric_limits<std::size_t>::max();

    const std::string answer = Generator::RandomString(StrLen);
    std::array<char, StrLen> buffer = {};
    auto chunks = SplitBufferToChunks<H, HPtr, Max, Factor>(buffer.begin(), buffer.end());
    auto partial_sums = PartialSums(chunks);

    const std::vector<std::size_t> counts = {
        0, 1,
        Max - 1, Max, Max + 1,
        chunks.size() - 1, chunks.size(),
    };
    const std::vector<std::size_t> thresholds = {
        Threshold,
        MaxThreshold,
    };

    for (const auto count : counts) {
        const std::size_t chars = partial_sums[count];

        for (const auto threshold : thresholds) {
            Reader::input.str(answer);

            Reader::threshold = threshold;
            Writer::threshold = threshold;
            
            TContIOVector<TTraits1, Reader> iovec_reader(chunks.data(), count);
            auto result = iovec_reader.TryProcessAllBytes(dummy_fd);
            EXPECT_TRUE(static_cast<std::size_t>(result.processed_bytes) == chars
                    && result.error == 0) << "Iovector error: unexpected result.";

            EXPECT_EQ(std::memcmp(buffer.data(), answer.data(), chars), 0);

            TContIOVector<TTraits2, Writer> iovec_writer(chunks.data(), count);
            result = iovec_writer.TryProcessAllBytes(dummy_fd);
            EXPECT_TRUE(static_cast<std::size_t>(result.processed_bytes) == chars
                    && result.error == 0) << "Iovector error: unexpected result.";

            std::string from_dst = Writer::output.str();

            EXPECT_EQ(std::memcmp(from_dst.data(), answer.data(), chars), 0);

            std::memset(buffer.data(), 0, buffer.size());
            Writer::output.str("");

            Reader::threshold = Reader::max_threshold;
            Writer::threshold = Writer::max_threshold;
        }
    }
}

template <typename TTraits>
void TestMake() {
    using H = typename TTraits::HighLevel;
    using L = typename TTraits::LowLevel;

    char value = Generator::RandomChar();
    std::size_t size = sizeof(char);

    typename H::Type cxx_span;
    cxx_span.buf = reinterpret_cast<H::TPtrBase>(ToCharPtr(&value));
    cxx_span.len = size;
    typename L::Type c_span = TTraits::Make(cxx_span);

    EXPECT_EQ(&value, ToCharPtr(L::PtrBase(c_span)));
    EXPECT_EQ(size, L::Length(c_span));

    char new_value = '\0';
    do {
        new_value = Generator::RandomChar();
    } while (new_value == value);
    
    L::PtrBase(c_span) = reinterpret_cast<L::TPtrBase>(&new_value);
    EXPECT_EQ(&new_value, ToCharPtr(L::PtrBase(c_span)));

    L::PtrBase(c_span) = reinterpret_cast<L::TPtrBase>(&value);
    *ToCharPtr(L::PtrBase(c_span)) = new_value;

    EXPECT_EQ(&value, ToCharPtr(L::PtrBase(c_span)));
    EXPECT_EQ(value, new_value);

    std::size_t new_size = Generator::RandomSize();
    L::Length(c_span) = new_size;
    EXPECT_EQ(new_size, L::Length(c_span));
}

template <typename TTraits,
          typename TExpectedSpan,
          typename TExpectedPtrBase, typename TExpectedLength>
void TestTraits(std::size_t expected_max_count,
                bool is_compatibility_known_in_compile_time,
                bool is_compatible_by_compile_time,
                bool is_compatible_by_runtime) noexcept {
    using HType = typename TTraits::HighLevel::Type;
    using HPtr = typename TTraits::HighLevel::TPtrBase;
    using HLen = typename TTraits::HighLevel::TLength;

    EXPECT_TRUE((std::is_same_v<HType, TExpectedSpan>));
    EXPECT_TRUE((std::is_same_v<HPtr, TExpectedPtrBase>));
    EXPECT_TRUE((std::is_same_v<HLen, TExpectedLength>));

    EXPECT_EQ(expected_max_count, TTraits::LowLevel::MaxCount);

    EXPECT_EQ(TTraits::IsCompatibilityKnownInCompileTime, is_compatibility_known_in_compile_time);
    EXPECT_EQ(TTraits::IsCompatibleByCompileTime, is_compatible_by_compile_time);
    EXPECT_EQ(TTraits::IsCompatibleByRuntime, is_compatible_by_runtime);

    TestMake<TTraits>();
}

TEST(IOVectorTest, Traits) {
    constexpr std::size_t Max = 5;

    TestTraits<TCompatTraits<void, Max>,
               TCompatSpan<void>, void*, std::size_t>(Max, true, true, true);
    TestTraits<TCompatTraits<char, Max>,
               TCompatSpan<char>, char*, std::size_t>(Max, true, true, true);
    TestTraits<TCompatTraits<unsigned char, Max>,
               TCompatSpan<unsigned char>, unsigned char*, std::size_t>(Max, true, true, true);
    TestTraits<TCompatTraits<std::byte, Max>,
               TCompatSpan<std::byte>, std::byte*, std::size_t>(Max, true, true, true);
    TestTraits<TCompatTraits<const void, Max>,
               TCompatSpan<const void>, const void*, std::size_t>(Max, true, true, true);
    TestTraits<TCompatTraits<const char, Max>,
               TCompatSpan<const char>, const char*, std::size_t>(Max, true, true, true);
    TestTraits<TCompatTraits<const unsigned char, Max>,
               TCompatSpan<const unsigned char>, const unsigned char*, std::size_t>(Max, true, true, true);
    TestTraits<TCompatTraits<const std::byte, Max>,
               TCompatSpan<const std::byte>, const std::byte*, std::size_t>(Max, true, true, true);

    TestTraits<TIncompatTraits<void, Max>,
               TIncompatSpan<void>, void*, std::size_t>(Max, true, false, false);
    TestTraits<TIncompatTraits<char, Max>,
               TIncompatSpan<char>, char*, std::size_t>(Max, true, false, false);
    TestTraits<TIncompatTraits<unsigned char, Max>,
               TIncompatSpan<unsigned char>, unsigned char*, std::size_t>(Max, true, false, false);
    TestTraits<TIncompatTraits<std::byte, Max>,
               TIncompatSpan<std::byte>, std::byte*, std::size_t>(Max, true, false, false);
    TestTraits<TIncompatTraits<const void, Max>,
               TIncompatSpan<const void>, const void*, std::size_t>(Max, true, false, false);
    TestTraits<TIncompatTraits<const char, Max>,
               TIncompatSpan<const char>, const char*, std::size_t>(Max, true, false, false);
    TestTraits<TIncompatTraits<const unsigned char, Max>,
               TIncompatSpan<const unsigned char>, const unsigned char*, std::size_t>(Max, true, false, false);
    TestTraits<TIncompatTraits<const std::byte, Max>,
               TIncompatSpan<const std::byte>, const std::byte*, std::size_t>(Max, true, false, false);

    TestTraits<TCompatStdLikeSpanTrains<void, Max>,
               TCompatStdLikeSpan<void>, void*, std::size_t>(Max, false, false, true);
    TestTraits<TCompatStdLikeSpanTrains<char, Max>,
               TCompatStdLikeSpan<char>, char*, std::size_t>(Max, false, false, true);
    TestTraits<TCompatStdLikeSpanTrains<unsigned char, Max>,
               TCompatStdLikeSpan<unsigned char>, unsigned char*, std::size_t>(Max, false, false, true);
    TestTraits<TCompatStdLikeSpanTrains<std::byte, Max>,
               TCompatStdLikeSpan<std::byte>, std::byte*, std::size_t>(Max, false, false, true);
    TestTraits<TCompatStdLikeSpanTrains<const void, Max>,
               TCompatStdLikeSpan<const void>, const void*, std::size_t>(Max, false, false, true);
    TestTraits<TCompatStdLikeSpanTrains<const char, Max>,
               TCompatStdLikeSpan<const char>, const char*, std::size_t>(Max, false, false, true);
    TestTraits<TCompatStdLikeSpanTrains<const unsigned char, Max>,
               TCompatStdLikeSpan<const unsigned char>, const unsigned char*, std::size_t>(Max, false, false, true);
    TestTraits<TCompatStdLikeSpanTrains<const std::byte, Max>,
               TCompatStdLikeSpan<const std::byte>, const std::byte*, std::size_t>(Max, false, false, true);

    TestTraits<TIncompatStdLikeSpanTrains<void, Max>,
               TIncompatStdLikeSpan<void>, void*, std::size_t>(Max, false, false, false);
    TestTraits<TIncompatStdLikeSpanTrains<char, Max>,
               TIncompatStdLikeSpan<char>, char*, std::size_t>(Max, false, false, false);
    TestTraits<TIncompatStdLikeSpanTrains<unsigned char, Max>,
               TIncompatStdLikeSpan<unsigned char>, unsigned char*, std::size_t>(Max, false, false, false);
    TestTraits<TIncompatStdLikeSpanTrains<std::byte, Max>,
               TIncompatStdLikeSpan<std::byte>, std::byte*, std::size_t>(Max, false, false, false);
    TestTraits<TIncompatStdLikeSpanTrains<const void, Max>,
               TIncompatStdLikeSpan<const void>, const void*, std::size_t>(Max, false, false, false);
    TestTraits<TIncompatStdLikeSpanTrains<const char, Max>,
               TIncompatStdLikeSpan<const char>, const char*, std::size_t>(Max, false, false, false);
    TestTraits<TIncompatStdLikeSpanTrains<const unsigned char, Max>,
               TIncompatStdLikeSpan<const unsigned char>, const unsigned char*, std::size_t>(Max, false, false, false);
    TestTraits<TIncompatStdLikeSpanTrains<const std::byte, Max>,
               TIncompatStdLikeSpan<const std::byte>, const std::byte*, std::size_t>(Max, false, false, false);
}

TEST(IOVectorTest, SmallChunks) {
    using TTraits = TCompatTraits<const char, 5>;

    std::array chunks = {
        TTraits::HighLevel::Type("Hello", 5),
        TTraits::HighLevel::Type("World", 5),
    };

    TContIOVector<TTraits, TMockReaderHandler<TTraits>> iovec(chunks.data(), chunks.size());
    
    EXPECT_EQ(ToCharPtr(TTraits::LowLevel::PtrBase(*iovec.Current())), chunks[0].buf);

    iovec.Proceed(5);
    EXPECT_EQ(ToCharPtr(TTraits::LowLevel::PtrBase(*iovec.Current())), chunks[1].buf);
}

TEST(IOVectorTest, ReadingThenWriting) {
    constexpr std::size_t Max = 5;

    TestIOVectorWithMockHandler<TCompatTraits<void, Max>>();
    TestIOVectorWithMockHandler<TIncompatTraits<void, Max>>();
    TestIOVectorWithMockHandler<TCompatStdLikeSpanTrains<void, Max>>();
    TestIOVectorWithMockHandler<TIncompatStdLikeSpanTrains<void, Max>>();
}
