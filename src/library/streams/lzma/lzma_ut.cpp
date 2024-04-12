#include "lzma.h"

#include <src/library/testing/unittest/registar.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/mem.h>
#include <src/util/random/fast.h>
#include <ydb-cpp-sdk/util/random/random.h>
=======
#include <src/util/stream/mem.h>
#include <src/util/random/fast.h>
#include <src/util/random/random.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

class TStrokaByOneByte: public IZeroCopyInput {
public:
    TStrokaByOneByte(const std::string& s)
        : Data(s)
        , Pos(s.data())
    {
    }

private:
    size_t DoNext(const void** ptr, size_t len) override {
        if (Pos < Data.end()) {
            len = Min(len, static_cast<size_t>(1));
            *ptr = Pos;
            Pos += len;
            return len;
        } else {
            return 0;
        }
    }

    std::string Data;
    const char* Pos;
};

class TLzmaTest: public TTestBase {
    UNIT_TEST_SUITE(TLzmaTest);
    UNIT_TEST(Test1)
    UNIT_TEST(Test2)
    UNIT_TEST_SUITE_END();

private:
    inline std::string GenData() {
        std::string data;
        TReallyFastRng32 rnd(RandomNumber<ui64>());

        for (size_t i = 0; i < 50000; ++i) {
            const char ch = rnd.Uniform(256);
            const size_t len = 1 + rnd.Uniform(10);

            data += std::string(len, ch);
        }

        return data;
    }

    inline void Test2() {
        class TExcOutput: public IOutputStream {
        public:
            ~TExcOutput() override {
            }

            void DoWrite(const void*, size_t) override {
                throw 12345;
            }
        };

        std::string data(GenData());
        TMemoryInput mi(data.data(), data.size());
        TExcOutput out;

        try {
            TLzmaCompress c(&out);

            TransferData(&mi, &c);
        } catch (int i) {
            UNIT_ASSERT_EQUAL(i, 12345);
        }
    }

    inline void Test1() {
        std::string data(GenData());
        std::string data1;
        std::string res;

        {
            TMemoryInput mi(data.data(), data.size());
            std::stringOutput so(res);
            TLzmaCompress c(&so);

            TransferData(&mi, &c);

            c.Finish();
        }

        {
            TMemoryInput mi(res.data(), res.size());
            std::stringOutput so(data1);
            TLzmaDecompress d((IInputStream*)&mi);

            TransferData(&d, &so);
        }

        UNIT_ASSERT_EQUAL(data, data1);

        data1.clear();
        {
            TMemoryInput mi(res.data(), res.size());
            std::stringOutput so(data1);
            TLzmaDecompress d(&mi);

            TransferData(&d, &so);
        }

        UNIT_ASSERT_EQUAL(data, data1);

        data1.clear();
        {
            TStrokaByOneByte mi(res);
            std::stringOutput so(data1);
            TLzmaDecompress d(&mi);

            TransferData(&d, &so);
        }

        UNIT_ASSERT_EQUAL(data, data1);
    }
};

UNIT_TEST_SUITE_REGISTRATION(TLzmaTest);
