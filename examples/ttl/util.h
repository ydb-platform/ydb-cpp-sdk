#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> origin/main
#include <ydb-cpp-sdk/client/table/table.h>

#include <src/util/folder/pathsplit.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
<<<<<<< HEAD
=======
#include <src/client/ydb_table/table.h>

#include <src/util/folder/pathsplit.h>
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main

namespace NExample {

using namespace NYdb;
using namespace NYdb::NTable;

class TYdbErrorException : public yexception {
public:
    TYdbErrorException(const TStatus& status)
        : Status(status) {}

    TStatus Status;
};

inline void ThrowOnError(const TStatus& status) {
    if (!status.IsSuccess()) {
        throw TYdbErrorException(status) << status;
    }
}

inline void PrintStatus(const TStatus& status) {
    std::cerr << "Status: " << ToString(status.GetStatus()) << std::endl;
    status.GetIssues().PrintTo(std::cerr);
}

inline std::string JoinPath(const std::string& basePath, const std::string& path) {
    if (basePath.empty()) {
        return path;
    }

    TPathSplitUnix prefixPathSplit(basePath);
    prefixPathSplit.AppendComponent(path);

    return prefixPathSplit.Reconstruct();
}

}
