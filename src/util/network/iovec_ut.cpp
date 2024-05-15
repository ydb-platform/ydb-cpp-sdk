#include "iovec.h"

#include <src/library/testing/gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <iterator>
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
                                && offsetof(TCompatSpan<T>, buf) == offsetof(iovec, iov_len)>;
#else // defined(_win_)
        TSpanAdapterTraits<WSABUF, &WSABUF::buf, &WSABUF::len, Max>
            ::template With<TCompatSpan<T>, &TCompatSpan<T>::buf, &TCompatSpan<T>::len,
                            offsetof(TCompatSpan<T>, buf) == offsetof(WSABUF, buf)
                                && offsetof(TCompatSpan<T>, buf) == offsetof(WSABUF, len)>;
#endif // defined(_win_)

template <typename T, std::size_t Max>
using TIncompatTraits =
#if !defined(_win_)
        TSpanAdapterTraits<iovec, &iovec::iov_base, &iovec::iov_len, Max>
            ::template With<TIncompatSpan<T>, &TIncompatSpan<T>::buf, &TIncompatSpan<T>::len,
                            offsetof(TIncompatSpan<T>, buf) == offsetof(iovec, iov_base)
                                && offsetof(TIncompatSpan<T>, buf) == offsetof(iovec, iov_len)>;
#else // defined(_win_)
        TSpanAdapterTraits<WSABUF, &WSABUF::buf, &WSABUF::len, Max>
            ::template With<TIncompatSpan<T>, &TIncompatSpan<T>::buf, &TIncompatSpan<T>::len,
                            offsetof(TIncompatSpan<T>, buf) == offsetof(WSABUF, buf)
                                && offsetof(TIncompatSpan<T>, buf) == offsetof(WSABUF, len)>;
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

template <typename TTraits>
struct TMockReaderHandler {
    static inline std::istringstream input;
    static inline auto threshold = std::numeric_limits<std::size_t>::max();

    ssize_t operator()(int, const TTraits::LowLevel::Type* iov, std::size_t count) const noexcept {
        using L = typename TTraits::LowLevel;

        const auto max_bytes = threshold;

        std::size_t bytes = 0;
        for (auto iter = iov, end = iov + count; iter != end; bytes += L::Length(*iter), ++iter) {
            const auto new_len = bytes + L::Length(*iter);
            if (new_len > max_bytes) {
                input.read(reinterpret_cast<char*>(L::PtrBase(*iter)),
                           static_cast<std::streamsize>(max_bytes - bytes));
                bytes += max_bytes - bytes;
                return static_cast<ssize_t>(bytes);
            }
            input.read(reinterpret_cast<char*>(L::PtrBase(*iter)),
                       static_cast<std::streamsize>(L::Length(*iter)));
        }
        return static_cast<ssize_t>(bytes);
    }
};

template <typename TTraits>
struct TMockWriterHandler {
    static inline std::ostringstream output;
    static inline auto threshold = std::numeric_limits<std::size_t>::max();

    ssize_t operator()(int, const TTraits::LowLevel::Type* iov, std::size_t count) const noexcept {
        using L = typename TTraits::LowLevel;

        const auto max_bytes = threshold;

        std::size_t bytes = 0;
        for (auto iter = iov, end = iov + count; iter != end; bytes += L::Length(*iter), ++iter) {
            const auto new_len = bytes + L::Length(*iter);
            if (new_len > max_bytes) {
                output.write(reinterpret_cast<char*>(L::PtrBase(*iter)),
                             static_cast<std::streamsize>(max_bytes - bytes));
                bytes += max_bytes - bytes;
                return static_cast<ssize_t>(bytes);
            }
            output.write(reinterpret_cast<char*>(L::PtrBase(*iter)),
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

template <typename TTraits1, typename TTraits2 = TTraits1>
void TestFileIOVector() {
    using H = typename TTraits1::HighLevel::Type;
    using HPtr = typename TTraits1::HighLevel::TPtrBase;

    constexpr std::size_t Max = TTraits1::LowLevel::MaxCount;
    constexpr std::size_t factor = 2;
    constexpr std::size_t str_len = factor * 10 * Max;

    const std::string answer = Generator::RandomString(str_len);
    std::array<char, str_len> buffer = {};
    std::vector<H> chunks = SplitBufferToChunks<H, HPtr, Max, factor>(buffer.begin(), buffer.end());

    std::ofstream output_stream("src.txt");
    EXPECT_TRUE(output_stream.is_open()) << "Cannot open file 'src.txt'.";
    output_stream << answer;
    output_stream.close();

    std::vector<std::size_t> partial_sums;
    partial_sums.reserve(chunks.size() + 1);
    std::size_t acc = 0;
    partial_sums.push_back(acc);
    for (auto iter = chunks.cbegin(), end = chunks.cend(); iter != end; ++iter) {
        partial_sums.push_back(partial_sums.back() + iter->len);
    }

    const std::vector<std::size_t> counts = {
        0, 1, Max - 1, Max, Max + 1, chunks.size() - 1, chunks.size(),
    };
    for (const auto count : counts) {
        std::size_t chars = partial_sums[count];

        TContIOVector<TTraits1, TReadVHandler> iovec_reader(chunks.data(), count);

        auto file = std::fopen("src.txt", "r");
        EXPECT_TRUE(file) << "Cannot open file 'src.txt'.";
        auto fd = fileno(file);
        auto result = iovec_reader.TryProcessAllBytes(fd);
        EXPECT_TRUE(static_cast<std::size_t>(result.processed_bytes) == chars
                && result.error == 0) << "Iovector error: unexpected result.";
        std::fclose(file);

        EXPECT_EQ(std::memcmp(buffer.data(), answer.data(), chars), 0);

        TContIOVector<TTraits2, TWriteVHandler> iovec_writer(chunks.data(), count);
        file = std::fopen("dst.txt", "w");
        EXPECT_TRUE(file) << "Cannot open file 'dst.txt'.";
        fd = fileno(file);
        result = iovec_writer.TryProcessAllBytes(fd);
        EXPECT_TRUE(static_cast<std::size_t>(result.processed_bytes) == chars
                && result.error == 0) << "Iovector error: unexpected result.";
        std::fclose(file);

        std::ifstream input_stream("dst.txt");
        std::string from_dst;
        from_dst.reserve(chars);
        EXPECT_TRUE(input_stream.is_open()) << "Cannot open file 'dst.txt'.";
        input_stream >> from_dst;
        input_stream.close();

        EXPECT_EQ(std::memcmp(from_dst.data(), answer.data(), chars), 0);

        std::memset(buffer.data(), 0, buffer.size());
    }
}

template <typename TTraits1, typename TTraits2 = TTraits1>
void TestMockIOVector() {
    using H = typename TTraits1::HighLevel::Type;
    using HPtr = typename TTraits1::HighLevel::TPtrBase;

    constexpr std::size_t Max = TTraits1::LowLevel::MaxCount;
    constexpr std::size_t factor = 2;
    constexpr std::size_t str_len = factor * 10 * Max;
    constexpr std::size_t threshold = str_len / 13;

    const std::string answer = Generator::RandomString(str_len);
    std::array<char, str_len> buffer = {};
    std::vector<H> chunks = SplitBufferToChunks<H, HPtr, Max, factor>(buffer.begin(), buffer.end());

    using Reader = TMockReaderHandler<TTraits1>;
    Reader::input.str(answer);
    Reader::threshold = threshold;

    const std::size_t chars = str_len;

    TContIOVector<TTraits1, Reader> iovec_reader(chunks.data(), chunks.size());

    auto result = iovec_reader.TryProcessAllBytes(1);
    EXPECT_TRUE(static_cast<std::size_t>(result.processed_bytes) == chars
            && result.error == 0) << "Iovector error: unexpected result.";

    EXPECT_EQ(std::memcmp(buffer.data(), answer.data(), chars), 0);

    using Writer = TMockWriterHandler<TTraits2>;
    Writer::threshold = threshold;

    TContIOVector<TTraits2, Writer> iovec_writer(chunks.data(), chunks.size());
    result = iovec_writer.TryProcessAllBytes(1);
    EXPECT_TRUE(static_cast<std::size_t>(result.processed_bytes) == chars
            && result.error == 0) << "Iovector error: unexpected result.";

    std::string from_dst = Writer::output.str();

    EXPECT_EQ(std::memcmp(from_dst.data(), answer.data(), chars), 0);

    std::memset(buffer.data(), 0, buffer.size());
    Writer::output.str("");
}

TEST(IOVectorTest, FileHandler) {
    constexpr std::size_t Max = 5;

    TestFileIOVector<TCompatTraits<void, Max>>();
    TestFileIOVector<TCompatTraits<char, Max>>();
    TestFileIOVector<TCompatTraits<unsigned char, Max>>();
    TestFileIOVector<TCompatTraits<std::byte, Max>>();
    TestFileIOVector<TCompatTraits<const void, Max>>();
    TestFileIOVector<TCompatTraits<const char, Max>>();
    TestFileIOVector<TCompatTraits<const unsigned char, Max>>();
    TestFileIOVector<TCompatTraits<const std::byte, Max>>();

    TestFileIOVector<TIncompatTraits<void, Max>>();
    TestFileIOVector<TIncompatTraits<char, Max>>();
    TestFileIOVector<TIncompatTraits<unsigned char, Max>>();
    TestFileIOVector<TIncompatTraits<std::byte, Max>>();
    TestFileIOVector<TIncompatTraits<const void, Max>>();
    TestFileIOVector<TIncompatTraits<const char, Max>>();
    TestFileIOVector<TIncompatTraits<const unsigned char, Max>>();
    TestFileIOVector<TIncompatTraits<const std::byte, Max>>();

    TestFileIOVector<TCompatStdLikeSpanTrains<void, Max>>();
    TestFileIOVector<TCompatStdLikeSpanTrains<char, Max>>();
    TestFileIOVector<TCompatStdLikeSpanTrains<unsigned char, Max>>();
    TestFileIOVector<TCompatStdLikeSpanTrains<std::byte, Max>>();
    TestFileIOVector<TCompatStdLikeSpanTrains<const void, Max>>();
    TestFileIOVector<TCompatStdLikeSpanTrains<const char, Max>>();
    TestFileIOVector<TCompatStdLikeSpanTrains<const unsigned char, Max>>();
    TestFileIOVector<TCompatStdLikeSpanTrains<const std::byte, Max>>();

    TestFileIOVector<TIncompatStdLikeSpanTrains<void, Max>>();
    TestFileIOVector<TIncompatStdLikeSpanTrains<char, Max>>();
    TestFileIOVector<TIncompatStdLikeSpanTrains<unsigned char, Max>>();
    TestFileIOVector<TIncompatStdLikeSpanTrains<std::byte, Max>>();
    TestFileIOVector<TIncompatStdLikeSpanTrains<const void, Max>>();
    TestFileIOVector<TIncompatStdLikeSpanTrains<const char, Max>>();
    TestFileIOVector<TIncompatStdLikeSpanTrains<const unsigned char, Max>>();
    TestFileIOVector<TIncompatStdLikeSpanTrains<const std::byte, Max>>();
}

TEST(IOVectorTest, MockHandlerWithPartialProcess) {
    constexpr std::size_t Max = 5;

    TestMockIOVector<TCompatTraits<void, Max>>();
    TestMockIOVector<TCompatTraits<char, Max>>();
    TestMockIOVector<TCompatTraits<unsigned char, Max>>();
    TestMockIOVector<TCompatTraits<std::byte, Max>>();
    TestMockIOVector<TCompatTraits<const void, Max>>();
    TestMockIOVector<TCompatTraits<const char, Max>>();
    TestMockIOVector<TCompatTraits<const unsigned char, Max>>();
    TestMockIOVector<TCompatTraits<const std::byte, Max>>();

    TestMockIOVector<TIncompatTraits<void, Max>>();
    TestMockIOVector<TIncompatTraits<char, Max>>();
    TestMockIOVector<TIncompatTraits<unsigned char, Max>>();
    TestMockIOVector<TIncompatTraits<std::byte, Max>>();
    TestMockIOVector<TIncompatTraits<const void, Max>>();
    TestMockIOVector<TIncompatTraits<const char, Max>>();
    TestMockIOVector<TIncompatTraits<const unsigned char, Max>>();
    TestMockIOVector<TIncompatTraits<const std::byte, Max>>();

    TestMockIOVector<TCompatStdLikeSpanTrains<void, Max>>();
    TestMockIOVector<TCompatStdLikeSpanTrains<char, Max>>();
    TestMockIOVector<TCompatStdLikeSpanTrains<unsigned char, Max>>();
    TestMockIOVector<TCompatStdLikeSpanTrains<std::byte, Max>>();
    TestMockIOVector<TCompatStdLikeSpanTrains<const void, Max>>();
    TestMockIOVector<TCompatStdLikeSpanTrains<const char, Max>>();
    TestMockIOVector<TCompatStdLikeSpanTrains<const unsigned char, Max>>();
    TestMockIOVector<TCompatStdLikeSpanTrains<const std::byte, Max>>();

    TestMockIOVector<TIncompatStdLikeSpanTrains<void, Max>>();
    TestMockIOVector<TIncompatStdLikeSpanTrains<char, Max>>();
    TestMockIOVector<TIncompatStdLikeSpanTrains<unsigned char, Max>>();
    TestMockIOVector<TIncompatStdLikeSpanTrains<std::byte, Max>>();
    TestMockIOVector<TIncompatStdLikeSpanTrains<const void, Max>>();
    TestMockIOVector<TIncompatStdLikeSpanTrains<const char, Max>>();
    TestMockIOVector<TIncompatStdLikeSpanTrains<const unsigned char, Max>>();
    TestMockIOVector<TIncompatStdLikeSpanTrains<const std::byte, Max>>();
}
