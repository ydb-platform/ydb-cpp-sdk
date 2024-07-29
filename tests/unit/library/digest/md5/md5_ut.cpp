#include "md5.h"

#include "registar.h"

#include <src/util/system/fs.h>
#include <src/util/stream/file.h>

Y_UNIT_TEST_SUITE(TMD5Test) {
    Y_UNIT_TEST(TestMD5) {
        // echo -n 'qwertyuiopqwertyuiopasdfghjklasdfghjkl' | md5sum
        constexpr const char* b = "qwertyuiopqwertyuiopasdfghjklasdfghjkl";

        MD5 r;
        r.Update((const unsigned char*)b, 15);
        r.Update((const unsigned char*)b + 15, strlen(b) - 15);

        char rs[33];
        std::string s(r.End(rs));
        std::ranges::transform(s, s.begin(), ::tolower);

        UNIT_ASSERT_NO_DIFF(s, std::string_view("3ac00dd696b966fd74deee3c35a59d8f"));

        std::string result = r.Calc(std::string_view(b));
        std::ranges::transform(result, result.begin(), ::tolower);
        UNIT_ASSERT_NO_DIFF(result, std::string_view("3ac00dd696b966fd74deee3c35a59d8f"));
    }

    Y_UNIT_TEST(TestFile) {
        std::string s = NUnitTest::RandomString(1000000, 1);
        const std::string tmpFile = "tmp";

        {
            TFixedBufferFileOutput fo(tmpFile);
            fo.Write(s.data(), s.size());
        }

        char fileBuf[100];
        char memBuf[100];
        std::string fileHash = MD5::File(tmpFile.data(), fileBuf);
        std::string memoryHash = MD5::Data((const unsigned char*)s.data(), s.size(), memBuf);

        UNIT_ASSERT_NO_DIFF(fileHash, memoryHash);

        fileHash = MD5::File(tmpFile);
        UNIT_ASSERT_NO_DIFF(fileHash, memoryHash);

        NFs::Remove(tmpFile);
        fileHash = MD5::File(tmpFile);
        UNIT_ASSERT_EQUAL(fileHash.size(), 0);
    }

    Y_UNIT_TEST(TestIsMD5) {
        UNIT_ASSERT_EQUAL(false, MD5::IsMD5(std::string_view()));
        UNIT_ASSERT_EQUAL(false, MD5::IsMD5(std::string_view("4136ebb0e4c45d21e2b09294c75cfa0")));   // length 31
        UNIT_ASSERT_EQUAL(false, MD5::IsMD5(std::string_view("4136ebb0e4c45d21e2b09294c75cfa000"))); // length 33
        UNIT_ASSERT_EQUAL(false, MD5::IsMD5(std::string_view("4136ebb0e4c45d21e2b09294c75cfa0g")));  // wrong character 'g'
        UNIT_ASSERT_EQUAL(true, MD5::IsMD5(std::string_view("4136EBB0E4C45D21E2B09294C75CFA08")));
        UNIT_ASSERT_EQUAL(true, MD5::IsMD5(std::string_view("4136ebb0E4C45D21e2b09294C75CfA08")));
        UNIT_ASSERT_EQUAL(true, MD5::IsMD5(std::string_view("4136ebb0e4c45d21e2b09294c75cfa08")));
    }

    Y_UNIT_TEST(TestMd5HalfMix) {
        UNIT_ASSERT_EQUAL(MD5::CalcHalfMix(""), 7203772011789518145ul);
        UNIT_ASSERT_EQUAL(MD5::CalcHalfMix("qwertyuiopqwertyuiopasdfghjklasdfghjkl"), 11753545595885642730ul);
    }
}
