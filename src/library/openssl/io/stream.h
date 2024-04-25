#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/stream/input.h>
#include <ydb-cpp-sdk/util/stream/output.h>
=======
#include <src/util/generic/ptr.h>
#include <src/util/stream/input.h>
#include <src/util/stream/output.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/stream/input.h>
#include <ydb-cpp-sdk/util/stream/output.h>
>>>>>>> origin/main

#include <optional>

class TOpenSslClientIO: public IInputStream, public IOutputStream {
public:
    struct TOptions {
        struct TVerifyCert {
            // Uses default CA path /etc/ssl/certs/ - can be provided with debian package: ca-certificates.deb.
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
    THolder<TImpl> Impl_;
};
