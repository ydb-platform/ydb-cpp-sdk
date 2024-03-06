#include "init.h"

#include <util/generic/singleton.h>
#include <util/generic/buffer.h>

#include <util/system/yassert.h>
#include <util/system/thread.h>

#include <util/random/entropy.h>
#include <util/stream/input.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/conf.h>
#include <openssl/crypto.h>

#include <mutex>
#include <vector>

namespace {
    struct TInitSsl {
        struct TOpensslLocks {
            inline TOpensslLocks()
                : Mutexes(CRYPTO_num_locks())
            {
                for (auto& mpref : Mutexes) {
                    mpref.reset(new std::mutex());
                }
            }

            inline void LockOP(int mode, int n) {
                auto& mutex = *Mutexes.at(n);

                if (mode & CRYPTO_LOCK) {
                    mutex.lock();
                } else {
                    mutex.unlock();
                }
            }

            std::vector<std::unique_ptr<std::mutex>> Mutexes;
        };

        inline TInitSsl() {
            OPENSSL_init_crypto(OPENSSL_INIT_NO_ATEXIT, nullptr);
        }

        inline ~TInitSsl() {
            OPENSSL_cleanup();
        }

        static void LockingFunction(int mode, int n, const char* /*file*/, int /*line*/) {
            Singleton<TOpensslLocks>()->LockOP(mode, n);
        }

        static unsigned long ThreadIdFunction() {
            return TThread::CurrentThreadId();
        }
    };
}

void InitOpenSSL() {
    (void)SingletonWithPriority<TInitSsl, 0>();
}
