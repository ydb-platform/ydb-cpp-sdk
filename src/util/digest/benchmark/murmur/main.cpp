#include <src/library/testing/gbenchmark/benchmark.h>

#include <src/util/digest/murmur.h>
<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/types.h>
=======
#include <src/util/system/types.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/types.h>
>>>>>>> origin/main

#include <array>

constexpr auto MakeTestData() {
    std::array<ui64, 4096> result{};
    for (ui64 i = 0; i < result.size(); ++i) {
        result[i] = i;
    }
    return result;
}

constexpr auto TEST_DATA = MakeTestData();

template <typename Result>
static void BenchmarkMurmurHash(benchmark::State& state) {
    for (auto _ : state) {
        Result hash = MurmurHash<Result>(TEST_DATA.data(), sizeof(TEST_DATA));
        Y_DO_NOT_OPTIMIZE_AWAY(hash);
    }
}

BENCHMARK_TEMPLATE(BenchmarkMurmurHash, ui32);
BENCHMARK_TEMPLATE(BenchmarkMurmurHash, ui64);
