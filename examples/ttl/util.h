#pragma once

#include <util/folder/pathsplit.h>
#include <util/string/printf.h>

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
    Cerr << "Status: " << status.GetStatus() << Endl;
    status.GetIssues().PrintTo(Cerr);
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
