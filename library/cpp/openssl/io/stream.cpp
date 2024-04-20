#include "stream.h"

#include <util/generic/singleton.h>
#include <util/generic/yexception.h>

#include <library/cpp/openssl/init/init.h>
#include <library/cpp/openssl/method/io.h>
#include <library/cpp/resource/resource.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/tls1.h>
#include <openssl/x509v3.h>

using TOptions = TOpenSslClientIO::TOptions;

namespace {
    struct TSslIO;

    struct TSslInitOnDemand {
        inline TSslInitOnDemand() {
            InitOpenSSL();
        }
    };

    int GetLastSslError() noexcept {
        return ERR_peek_last_error();
    }

    const char* SslErrorText(int error) noexcept {
        return ERR_error_string(error, nullptr);
    }

    inline std::string_view SslLastError() noexcept {
        return SslErrorText(GetLastSslError());
    }

    struct TSslError: public yexception {
        inline TSslError() {
            *this << SslLastError();
        }
    };

    /*struct TSslDestroy {
        static inline void Destroy(ssl_ctx_st* ctx) noexcept {
            SSL_CTX_free(ctx);
        }

        static inline void Destroy(ssl_st* ssl) noexcept {
            SSL_free(ssl);
        }

        static inline void Destroy(bio_st* bio) noexcept {
            BIO_free(bio);
        }

        static inline void Destroy(x509_st* x509) noexcept {
            X509_free(x509);
        }
    };*/
    struct TSslDestroy {
    void operator()(ssl_ctx_st* ctx) const noexcept {
        SSL_CTX_free(ctx);
    }

    void operator()(ssl_st* ssl) const noexcept {
        SSL_free(ssl);
    }

    void operator()(bio_st* bio) const noexcept {
        BIO_free(bio);
    }

    void operator()(x509_st* x509) const noexcept {
        X509_free(x509);
    }
};


    template <class T>
    using TSslHolderPtr = std::unique_ptr<T, TSslDestroy>;

    using TSslContextPtr = TSslHolderPtr<ssl_ctx_st>;
    using TSslPtr = TSslHolderPtr<ssl_st>;
    using TBioPtr = TSslHolderPtr<bio_st>;
    using TX509Ptr = TSslHolderPtr<x509_st>;

    inline TSslContextPtr CreateSslCtx(const ssl_method_st* method) {
        TSslContextPtr ctx(SSL_CTX_new(method));

        if (!ctx) {
            ythrow TSslError() << "SSL_CTX_new";
        }

        SSL_CTX_set_options(ctx.get(), SSL_OP_NO_SSLv2);
        SSL_CTX_set_options(ctx.get(), SSL_OP_NO_SSLv3);
        SSL_CTX_set_options(ctx.get(), SSL_OP_MICROSOFT_SESS_ID_BUG);
        SSL_CTX_set_options(ctx.get(), SSL_OP_NETSCAPE_CHALLENGE_BUG);

        return ctx;
    }

    struct TStreamIO : public NOpenSSL::TAbstractIO {
        inline TStreamIO(IInputStream* in, IOutputStream* out)
            : In(in)
            , Out(out)
        {
        }

        int Write(const char* data, size_t dlen, size_t* written) override {
            Out->Write(data, dlen);

            *written = dlen;
            return 1;
        }

        int Read(char* data, size_t dlen, size_t* readbytes) override {
            *readbytes = In->Read(data, dlen);
            return 1;
        }

        int Puts(const char* buf) override {
            Y_UNUSED(buf);
            return -1;
        }

        int Gets(char* buf, int size) override {
            Y_UNUSED(buf);
            Y_UNUSED(size);
            return -1;
        }

        void Flush() override {
        }

        IInputStream* In;
        IOutputStream* Out;
    };

    struct TSslIO: public TSslInitOnDemand, public TOptions {
        inline TSslIO(IInputStream* in, IOutputStream* out, const TOptions& opts)
            : TOptions(opts)
            , Io(in, out)
            , Ctx(CreateClientContext())
            , Ssl(ConstructSsl())
        {
            Connect();
        }

        inline TSslContextPtr CreateClientContext() {
            TSslContextPtr ctx = CreateSslCtx(SSLv23_client_method());
            if (ClientCert_) {
                if (ClientCert_->CertificateFile_.empty() || ClientCert_->PrivateKeyFile_.empty()) {
                    ythrow yexception() << "both client certificate and private key are required";
                }
                if (!ClientCert_->PrivateKeyPassword_.empty()) {
                    SSL_CTX_set_default_passwd_cb(ctx.get(), [](char* buf, int size, int rwflag, void* userData) -> int {
                        Y_UNUSED(rwflag);
                        auto io = static_cast<TSslIO*>(userData);
                        if (!io) {
                            return -1;
                        }
                        if (size < static_cast<int>(io->ClientCert_->PrivateKeyPassword_.size())) {
                            return -1;
                        }
                        return io->ClientCert_->PrivateKeyPassword_.copy(buf, size, 0);
                    });
                    SSL_CTX_set_default_passwd_cb_userdata(ctx.get(), this);
                }
                if (1 != SSL_CTX_use_certificate_chain_file(ctx.get(), ClientCert_->CertificateFile_.c_str())) {
                    ythrow TSslError() << "SSL_CTX_use_certificate_chain_file";
                }
                if (1 != SSL_CTX_use_PrivateKey_file(ctx.get(), ClientCert_->PrivateKeyFile_.c_str(), SSL_FILETYPE_PEM)) {
                    ythrow TSslError() << "SSL_CTX_use_PrivateKey_file";
                }
                if (1 != SSL_CTX_check_private_key(ctx.get())) {
                    ythrow TSslError() << "SSL_CTX_check_private_key (client)";
                }
            }
            return ctx;
        }

        inline TSslPtr ConstructSsl() {
            TSslPtr ssl(SSL_new(Ctx.get()));

            if (!ssl) {
                ythrow TSslError() << "SSL_new";
            }

            if (VerifyCert_) {
                InitVerification(ssl.get());
            }

            BIO_up_ref(Io); // SSL_set_bio consumes only one reference if rbio and wbio are the same
            SSL_set_bio(ssl.get(), Io, Io);

            return ssl;
        }

        inline void InitVerification(ssl_st* ssl) {
            X509_VERIFY_PARAM* param = SSL_get0_param(ssl);
            X509_VERIFY_PARAM_set_hostflags(param, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
            Y_ENSURE(X509_VERIFY_PARAM_set1_host(param, VerifyCert_->Hostname_.data(), VerifyCert_->Hostname_.size()));
            SSL_set_tlsext_host_name(ssl, VerifyCert_->Hostname_.data()); // TLS extenstion: SNI

            Y_ENSURE_EX(1 == SSL_CTX_set_default_verify_paths(Ctx.get()),
                        TSslError());
            // it is OK to ignore result of SSL_CTX_load_verify_locations():
            // Dir "/etc/ssl/certs/" may be missing
            SSL_CTX_load_verify_locations(Ctx.get(),
                                          "/etc/ssl/certs/ca-certificates.crt",
                                          "/etc/ssl/certs/");

            SSL_set_verify(ssl, SSL_VERIFY_PEER, nullptr);
        }

        inline void Connect() {
            if (SSL_connect(Ssl.get()) != 1) {
                ythrow TSslError() << "SSL_connect";
            }
        }

        inline void Finish() const {
            SSL_shutdown(Ssl.get());
        }

        inline size_t Read(void* buf, size_t len) {
            const int ret = SSL_read(Ssl.get(), buf, len);

            if (ret < 0) {
                ythrow TSslError() << "SSL_read";
            }

            return ret;
        }

        inline void Write(const char* buf, size_t len) {
            while (len) {
                const int ret = SSL_write(Ssl.get(), buf, len);

                if (ret < 0) {
                    ythrow TSslError() << "SSL_write";
                }

                buf += (size_t)ret;
                len -= (size_t)ret;
            }
        }

        TStreamIO Io;
        TSslContextPtr Ctx;
        TSslPtr Ssl;
    };
}

struct TOpenSslClientIO::TImpl: public TSslIO {
    inline TImpl(IInputStream* in, IOutputStream* out, const TOptions& opts)
        : TSslIO(in, out, opts)
    {
    }
};

TOpenSslClientIO::TOpenSslClientIO(IInputStream* in, IOutputStream* out)
    : Impl_(new TImpl(in, out, TOptions()))
{
}

TOpenSslClientIO::TOpenSslClientIO(IInputStream* in, IOutputStream* out, const TOptions& options)
    : Impl_(new TImpl(in, out, options))
{
}

TOpenSslClientIO::~TOpenSslClientIO() {
    try {
        Impl_->Finish();
    } catch (...) {
    }
}

void TOpenSslClientIO::DoWrite(const void* buf, size_t len) {
    Impl_->Write((const char*)buf, len);
}

size_t TOpenSslClientIO::DoRead(void* buf, size_t len) {
    return Impl_->Read(buf, len);
}
