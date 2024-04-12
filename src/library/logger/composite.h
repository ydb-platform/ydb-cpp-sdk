#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/logger/backend.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include "backend.h"
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))


class TCompositeLogBackend: public TLogBackend {
public:
    virtual void WriteData(const TLogRecord& rec) override;
    virtual void ReopenLog() override;
    virtual void AddLogBackend(THolder<TLogBackend>&& backend);

private:
    std::vector<THolder<TLogBackend>> Slaves;
    ELogPriority LogPriority = static_cast<ELogPriority>(0); // has now it's own priority by default
};
