#pragma once

#include "backend.h"
#include <util/generic/vector.h>


class TCompositeLogBackend: public TLogBackend {
public:
    virtual void WriteData(const TLogRecord& rec) override;
    virtual void ReopenLog() override;
    virtual void AddLogBackend(std::unique_ptr<TLogBackend>&& backend);

private:
    std::vector<std::unique_ptr<TLogBackend>> Slaves;
    ELogPriority LogPriority = static_cast<ELogPriority>(0); // has now it's own priority by default
};
