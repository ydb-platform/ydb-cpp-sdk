#include <ydb-cpp-sdk/library/logger/backend.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/generic/singleton.h>
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <mutex>

namespace {
    class TGlobalLogsStorage {
    private:
        std::vector<TLogBackend*> Backends;
        std::mutex Mutex;

    public:
        void Register(TLogBackend* backend) {
            std::lock_guard g(Mutex);
            Backends.push_back(backend);
        }

        void UnRegister(TLogBackend* backend) {
            std::lock_guard g(Mutex);
            for (ui32 i = 0; i < Backends.size(); ++i) {
                if (Backends[i] == backend) {
                    Backends.erase(Backends.begin() + i);
                    return;
                }
            }
            Y_ABORT("Incorrect pointer for log backend");
        }

        void Reopen(bool flush) {
            std::lock_guard g(Mutex);
            for (auto& b : Backends) {
                if (typeid(*b) == typeid(TLogBackend)) {
                    continue;
                }
                if (flush) {
                    b->ReopenLog();
                } else {
                    b->ReopenLogNoFlush();
                }
            }
        }
    };
}

template <>
class TSingletonTraits<TGlobalLogsStorage> {
public:
    static const size_t Priority = 50;
};

ELogPriority TLogBackend::FiltrationLevel() const {
    return LOG_MAX_PRIORITY;
}

TLogBackend::TLogBackend() noexcept {
    Singleton<TGlobalLogsStorage>()->Register(this);
}

TLogBackend::~TLogBackend() {
    Singleton<TGlobalLogsStorage>()->UnRegister(this);
}

void TLogBackend::ReopenLogNoFlush() {
    ReopenLog();
}

void TLogBackend::ReopenAllBackends(bool flush) {
    Singleton<TGlobalLogsStorage>()->Reopen(flush);
}

size_t TLogBackend::QueueSize() const {
    ythrow yexception() << "Not implemented.";
}
