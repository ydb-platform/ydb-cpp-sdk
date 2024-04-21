#include <src/library/testing/gtest/matchers.h>

#include <src/library/testing/common/env.h>

#include <src/util/folder/path.h>
#include <src/util/stream/file.h>
#include <src/util/system/fs.h>

bool NGTest::NDetail::MatchOrUpdateGolden(std::string_view actualContent, const std::string& goldenFilename) {
    if (!GetTestParam("GTEST_UPDATE_GOLDEN").empty()) {
        Y_ENSURE(NFs::MakeDirectoryRecursive(TFsPath(goldenFilename).Parent()));
        TFile file(goldenFilename, CreateAlways);
        file.Write(actualContent.data(), actualContent.size());
        std::cerr << "The data[" << actualContent.size() << "] has written to golden file " << goldenFilename << std::endl;
        return true;
    }

    if (!NFs::Exists(goldenFilename)) {
        return actualContent.empty();
    }
    TFile goldenFile(goldenFilename, RdOnly);
    return actualContent == TFileInput(goldenFile).ReadAll();
}
