#pragma once

#include <util/generic/noncopyable.h>
#include <util/generic/ptr.h>

namespace NPar {

struct ILocallyExecutable : virtual public TThrRefBase {
    virtual void LocalExec(int id) = 0;
};

class ILocalExecutor : public TNonCopyable {
public:
    enum EFlags : int {
        HIGH_PRIORITY = 0,
        MED_PRIORITY = 1,
        LOW_PRIORITY = 2,
        PRIORITY_MASK = 3,
        WAIT_COMPLETE = 4,
    };

    virtual ~ILocalExecutor() = default;

    virtual void Exec(TIntrusivePtr<ILocallyExecutable> exec, int id, int flags) = 0;
    virtual void ExecRange(TIntrusivePtr<ILocallyExecutable> exec, int firstId, int lastId, int flags) = 0;
    virtual int GetWorkerThreadId() const noexcept = 0;
    virtual int GetThreadCount() const noexcept = 0;
};

} // namespace NPar
