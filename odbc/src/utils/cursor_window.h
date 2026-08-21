#pragma once

#include <sql.h>

#include <cstddef>
#include <optional>

namespace NYdb::NOdbc {

struct TFetchResult {
    SQLULEN Rows = 0;
    bool OverlappedStart = false;
};

class TCursorWindow {
public:
    explicit TCursorWindow(size_t totalRows = 0);

    TFetchResult Fetch(
        SQLSMALLINT orientation,
        SQLLEN offset,
        SQLULEN rowsetSize,
        SQLULEN maxRows);
    std::optional<size_t> Resolve(SQLULEN row) const;

private:
    enum class EPosition {
        Before,
        Rowset,
        After,
    };

    void SetBoundary(EPosition position);

    size_t TotalRows_ = 0;
    EPosition Position_ = EPosition::Before;
    size_t Start_ = 0;
    size_t Size_ = 0;
    size_t PreviousRowsetSize_ = 0;
};

} // namespace NYdb::NOdbc
