#include "http_digest.h"

#include <src/library/digest/md5/md5.h>
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/stream/str.h>
=======
#include <src/util/stream/output.h>
#include <src/util/stream/str.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

/************************************************************/
/************************************************************/
static const char* WWW_PREFIX = "Authorization: Digest ";

/************************************************************/
httpDigestHandler::httpDigestHandler()
    : User_(nullptr)
    , Password_(nullptr)
    , Nonce_(nullptr)
    , NonceCount_(0)
    , HeaderInstruction_(nullptr)
{
}

/************************************************************/
httpDigestHandler::~httpDigestHandler() {
    clear();
}

/************************************************************/
void httpDigestHandler::clear() {
    free(Nonce_);
    free(HeaderInstruction_);
    User_ = Password_ = nullptr;
    Nonce_ = HeaderInstruction_ = nullptr;
    NonceCount_ = 0;
}

/************************************************************/
void httpDigestHandler::setAuthorization(const char* user, const char* password) {
    clear();
    if (user && password) {
        User_ = user;
        Password_ = password;
    }
}

/************************************************************/
const char* httpDigestHandler::getHeaderInstruction() const {
    return HeaderInstruction_;
}

/************************************************************/
void httpDigestHandler::generateCNonce(char* outCNonce, size_t outCNonceSize) {
    if (!*outCNonce)
        snprintf(outCNonce, outCNonceSize, "%ld", (long)time(nullptr));
}

/************************************************************/
inline void addMD5(MD5& ctx, const char* value) {
    ctx.Update((const unsigned char*)(value), strlen(value));
}

inline void addMD5(MD5& ctx, const char* value, int len) {
    ctx.Update((const unsigned char*)(value), len);
}

inline void addMD5Sep(MD5& ctx) {
    addMD5(ctx, ":", 1);
}

/************************************************************/
/* calculate H(A1) as per spec */
void httpDigestHandler::digestCalcHA1(const THttpAuthHeader& hd,
                                      char* outSessionKey,
                                      char* outCNonce,
                                      size_t outCNonceSize) {
    MD5 ctx;
    ctx.Init();
    addMD5(ctx, User_);
    addMD5Sep(ctx);
    addMD5(ctx, hd.realm);
    addMD5Sep(ctx);
    addMD5(ctx, Password_);

    if (hd.algorithm == 1) { //MD5-sess
        unsigned char digest[16];
        ctx.Final(digest);

        generateCNonce(outCNonce, outCNonceSize);

        ctx.Init();
        ctx.Update(digest, 16);
        addMD5Sep(ctx);
        addMD5(ctx, hd.nonce);
        addMD5Sep(ctx);
        addMD5(ctx, outCNonce);
        ctx.End(outSessionKey);
    }

    ctx.End(outSessionKey);
}

/************************************************************/
/* calculate request-digest/response-digest as per HTTP Digest spec */
void httpDigestHandler::digestCalcResponse(const THttpAuthHeader& hd,
                                           const char* path,
                                           const char* method,
                                           const char* nonceCount,
                                           char* outResponse,
                                           char* outCNonce,
                                           size_t outCNonceSize) {
    char HA1[33];
    digestCalcHA1(hd, HA1, outCNonce, outCNonceSize);

    char HA2[33];
    MD5 ctx;
    ctx.Init();
    addMD5(ctx, method);
    addMD5Sep(ctx);
    addMD5(ctx, path);
    //ignore auth-int
    ctx.End(HA2);

    ctx.Init();
    addMD5(ctx, HA1, 32);
    addMD5Sep(ctx);
    addMD5(ctx, Nonce_);
    addMD5Sep(ctx);

    if (hd.qop_auth) {
        if (!*outCNonce)
            generateCNonce(outCNonce, outCNonceSize);

        addMD5(ctx, nonceCount, 8);
        addMD5Sep(ctx);
        addMD5(ctx, outCNonce);
        addMD5Sep(ctx);
        addMD5(ctx, "auth", 4);
        addMD5Sep(ctx);
    }
    addMD5(ctx, HA2, 32);
    ctx.End(outResponse);
}

/************************************************************/
bool httpDigestHandler::processHeader(const THttpAuthHeader* header,
                                      const char* path,
                                      const char* method,
                                      const char* cnonce) {
    if (!User_ || !header || !header->use_auth || !header->realm || !header->nonce)
        return false;

    if (Nonce_) {
        if (strcmp(Nonce_, header->nonce)) {
            free(Nonce_);
            Nonce_ = nullptr;
            NonceCount_ = 0;
        }
    }
    if (!Nonce_) {
        Nonce_ = strdup(header->nonce);
        NonceCount_ = 0;
    }
    free(HeaderInstruction_);
    HeaderInstruction_ = nullptr;
    NonceCount_++;

    char nonceCount[20];
    snprintf(nonceCount, sizeof(nonceCount), "%08d", NonceCount_);

    char CNonce[50];
    if (cnonce)
        strcpy(CNonce, cnonce);
    else
        CNonce[0] = 0;

    char response[33];
    digestCalcResponse(*header, path, method, nonceCount, response, CNonce, sizeof(CNonce));

    //digest-response  = 1#( username | realm | nonce | digest-uri
    //                   | response | [ algorithm ] | [cnonce] |
    //                   [opaque] | [message-qop] |
    //                   [nonce-count]  | [auth-param] )

    TStringStream out;
    out << WWW_PREFIX << "username=\"" << User_ << "\"";
    out << ", realm=\"" << header->realm << "\"";
    out << ", nonce=\"" << header->nonce << "\"";
    out << ", uri=\"" << path << "\"";
    if (header->algorithm == 1)
        out << ", algorithm=MD5-sess";
    else
        out << ", algorithm=MD5";
    if (header->qop_auth)
        out << ", qop=auth";
    out << ", nc=" << nonceCount;
    if (CNonce[0])
        out << ", cnonce=\"" << CNonce << "\"";
    out << ", response=\"" << response << "\"";
    if (header->opaque)
        out << ", opaque=\"" << header->opaque << "\"";
    out << "\r\n";

    std::string s_out = out.Str();
    HeaderInstruction_ = strdup(s_out.c_str());

    return true;
}

/************************************************************/
/************************************************************/
