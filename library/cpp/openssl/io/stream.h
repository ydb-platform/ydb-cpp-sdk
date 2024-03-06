#pragma once


#include <util/stream/input.h>
#include <util/stream/output.h>

#include <optional>

class TOpenSslClientIO: public IInputStream, public IOutputStream {
public:
    struct TOptions {
        struct TVerifyCert {
            // Uses builtin certs.
            // Also uses default CA path /etc/ssl/certs/ - can be provided with debian package: ca-certificates.deb.
            // It can be expanded with ENV: SSL_CERT_DIR.
            std::string Hostname_;
        };
        struct TClientCert {
            std::string CertificateFile_;
            std::string PrivateKeyFile_;
            std::string PrivateKeyPassword_;
        };

        std::optional<TVerifyCert> VerifyCert_;
        std::optional<TClientCert> ClientCert_;
        // TODO - keys, cyphers, etc
    };

    TOpenSslClientIO(IInputStream* in, IOutputStream* out);
    TOpenSslClientIO(IInputStream* in, IOutputStream* out, const TOptions& options);
    ~TOpenSslClientIO() override;

private:
    void DoWrite(const void* buf, size_t len) override;
    size_t DoRead(void* buf, size_t len) override;

private:
    struct TImpl;
    std::unique_ptr<TImpl> Impl_;
};

struct x509_store_st;

namespace NPrivate {
    struct TSslDestroy {
        static void Destroy(x509_store_st* x509) noexcept;
        void operator() (x509_store_st* x509) noexcept;
    };
}

using TOpenSslX509StorePtr = std::unique_ptr<x509_store_st, NPrivate::TSslDestroy>;
TOpenSslX509StorePtr GetBuiltinOpenSslX509Store();
