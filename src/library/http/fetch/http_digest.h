#pragma once

#include <ydb-cpp-sdk/library/http/fetch/httpheader.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/compat.h>
#include <ydb-cpp-sdk/library/http/misc/httpcodes.h>
=======
#include <src/util/system/compat.h>
#include <src/library/http/misc/httpcodes.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

class httpDigestHandler {
protected:
    const char* User_;
    const char* Password_;
    char* Nonce_;
    int NonceCount_;
    char* HeaderInstruction_;

    void clear();

    void generateCNonce(char* outCNonce, size_t outCNonceSize);

    void digestCalcHA1(const THttpAuthHeader& hd,
                       char* outSessionKey,
                       char* outCNonce,
                       size_t outCNonceSize);

    void digestCalcResponse(const THttpAuthHeader& hd,
                            const char* method,
                            const char* path,
                            const char* nonceCount,
                            char* outResponse,
                            char* outCNonce,
                            size_t outCNonceSize);

public:
    httpDigestHandler();
    ~httpDigestHandler();

    void setAuthorization(const char* user,
                          const char* password);
    bool processHeader(const THttpAuthHeader* header,
                       const char* path,
                       const char* method,
                       const char* cnonce = nullptr);

    bool empty() const {
        return (!User_);
    }

    const char* getHeaderInstruction() const;
};
