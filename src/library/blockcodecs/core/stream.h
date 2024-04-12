#pragma once

#include <src/util/stream/walk.h>
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/input.h>
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/stream/zerocopy.h>
=======
#include <src/util/stream/input.h>
#include <src/util/stream/output.h>
#include <src/util/stream/zerocopy.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
#include <src/util/generic/buffer.h>

namespace NBlockCodecs {
    struct ICodec;

    class TCodedOutput: public IOutputStream {
    public:
        TCodedOutput(IOutputStream* out, const ICodec* c, size_t bufLen);
        ~TCodedOutput() override;

    private:
        void DoWrite(const void* buf, size_t len) override;
        void DoFlush() override;
        void DoFinish() override;

        bool FlushImpl();

    private:
        const ICodec* C_;
        TBuffer D_;
        TBuffer O_;
        IOutputStream* S_;
    };

    class TDecodedInput: public IWalkInput {
    public:
        TDecodedInput(IInputStream* in);
        TDecodedInput(IInputStream* in, const ICodec* codec);

        ~TDecodedInput() override;

    private:
        size_t DoUnboundedNext(const void** ptr) override;

    private:
        TBuffer D_;
        IInputStream* S_;
        const ICodec* C_;
    };
}
