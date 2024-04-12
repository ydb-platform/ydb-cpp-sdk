#include "fs.h"
#include "fs_win.h"

#include <src/library/testing/unittest/registar.h>

#include "fileapi.h"

#include "file.h"
#include "fstat.h"
#include "win_undef.h"
#include <src/util/charset/wide.h>
#include <src/util/folder/path.h>

static void Touch(const TFsPath& path) {
    TFile file(path, CreateAlways | WrOnly);
    file.Write("1115", 4);
}

static LPCWSTR UTF8ToWCHAR(const std::string_view str, std::u16string& wstr) {
    wstr.resize(str.size());
    size_t written = 0;
    if (!UTF8ToWide(str.data(), str.size(), wstr.begin(), written))
        return nullptr;
    wstr.erase(written);
    static_assert(sizeof(WCHAR) == sizeof(wchar16), "expect sizeof(WCHAR) == sizeof(wchar16)");
    return (const WCHAR*)wstr.data();
}

Y_UNIT_TEST_SUITE(TFsWinTest) {
    Y_UNIT_TEST(TestRemoveDirWithROFiles) {
        TFsPath dir1 = "dir1";
        NFsPrivate::WinRemove(dir1);
        UNIT_ASSERT(!NFsPrivate::WinExists(dir1));
        UNIT_ASSERT(NFsPrivate::WinMakeDirectory(dir1));

        UNIT_ASSERT(TFileStat(dir1).IsDir());
        TFsPath file1 = dir1 / "file.txt";
        Touch(file1);
        UNIT_ASSERT(NFsPrivate::WinExists(file1));
        {
            std::u16string wstr;
            LPCWSTR wname = UTF8ToWCHAR(static_cast<const std::string&>(file1), wstr);
            UNIT_ASSERT(wname);
            WIN32_FILE_ATTRIBUTE_DATA fad;
            fad.dwFileAttributes = FILE_ATTRIBUTE_READONLY;
            ::SetFileAttributesW(wname, fad.dwFileAttributes);
        }
        NFsPrivate::WinRemove(dir1);
        UNIT_ASSERT(!NFsPrivate::WinExists(dir1));
    }

    Y_UNIT_TEST(TestRemoveReadOnlyDir) {
        TFsPath dir1 = "dir1";
        NFsPrivate::WinRemove(dir1);
        UNIT_ASSERT(!NFsPrivate::WinExists(dir1));
        UNIT_ASSERT(NFsPrivate::WinMakeDirectory(dir1));

        UNIT_ASSERT(TFileStat(dir1).IsDir());
        {
            std::u16string wstr;
            LPCWSTR wname = UTF8ToWCHAR(static_cast<const std::string&>(dir1), wstr);
            UNIT_ASSERT(wname);
            WIN32_FILE_ATTRIBUTE_DATA fad;
            fad.dwFileAttributes = FILE_ATTRIBUTE_READONLY;
            ::SetFileAttributesW(wname, fad.dwFileAttributes);
        }
        NFsPrivate::WinRemove(dir1);
        UNIT_ASSERT(!NFsPrivate::WinExists(dir1));
    }
}
