#pragma once

#include <ydb-cpp-sdk/library/logger/backend.h>

class TNullLogBackend: public TLogBackend {
public:
    TNullLogBackend();
    ~TNullLogBackend() override;

    void WriteData(const TLogRecord& rec) override;
    void ReopenLog() override;
};
