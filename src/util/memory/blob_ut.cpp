#include "blob.h"

#include <src/library/testing/unittest/registar.h>

#include <src/util/system/tempfile.h>
#include <src/util/folder/path.h>
#include <src/util/stream/output.h>
#include <src/util/stream/file.h>
#include <src/util/generic/buffer.h>

#include <span>

Y_UNIT_TEST_SUITE(TBlobTest) {
    Y_UNIT_TEST(TestSubBlob) {
        TBlob child;
        const char* p = nullptr;

        {
            TBlob parent = TBlob::CopySingleThreaded("0123456789", 10);
            UNIT_ASSERT_EQUAL(parent.Length(), 10);
            p = parent.AsCharPtr();
            UNIT_ASSERT_EQUAL(memcmp(p, "0123456789", 10), 0);
            child = parent.SubBlob(2, 5);
        } // Don't worry about parent

        UNIT_ASSERT_EQUAL(child.Length(), 3);
        UNIT_ASSERT_EQUAL(memcmp(child.AsCharPtr(), "234", 3), 0);
        UNIT_ASSERT_EQUAL(p + 2, child.AsCharPtr());
    }

    Y_UNIT_TEST(TestFromStream) {
        std::string s("sjklfgsdyutfuyas54fa78s5f89a6df790asdf7");
        TMemoryInput mi(s.data(), s.size());
        TBlob b = TBlob::FromStreamSingleThreaded(mi);

        UNIT_ASSERT_EQUAL(std::string((const char*)b.Data(), b.Length()), s);
    }

    Y_UNIT_TEST(TestFromString) {
        std::string s("dsfkjhgsadftusadtf");
        TBlob b(TBlob::FromString(s));

        UNIT_ASSERT_EQUAL(std::string((const char*)b.Data(), b.Size()), s);
        const auto expectedRef = std::span<const ui8>{(ui8*)s.data(), s.size()};
        UNIT_ASSERT_EQUAL(std::span<const ui8>{b}, expectedRef);
    }

    Y_UNIT_TEST(TestFromBuffer) {
        const size_t sz = 1234u;
        TBuffer buf;
        buf.Resize(sz);
        UNIT_ASSERT_EQUAL(buf.Size(), sz);
        TBlob b = TBlob::FromBuffer(buf);
        UNIT_ASSERT_EQUAL(buf.Size(), 0u);
        UNIT_ASSERT_EQUAL(b.Size(), sz);
    }

    Y_UNIT_TEST(TestFromFile) {
        std::string path = "testfile";

        TOFStream stream(path);
        stream.Write("1234", 4);
        stream.Finish();

        auto testMode = [](TBlob blob) {
            UNIT_ASSERT_EQUAL(blob.Size(), 4);
            UNIT_ASSERT_EQUAL(std::string_view(static_cast<const char*>(blob.Data()), 4), "1234");
        };

        testMode(TBlob::FromFile(path));
        testMode(TBlob::PrechargedFromFile(path));
        testMode(TBlob::LockedFromFile(path));
    }

    Y_UNIT_TEST(TestEmptyLockedFiles) {
        std::string path = MakeTempName();
        TFsPath(path).Touch();
        TBlob::LockedFromFile(path);
    }
}
