#pragma once

#include <ydb-cpp-sdk/util/stream/input.h>
#include <ydb-cpp-sdk/util/stream/output.h>

#include <ydb-cpp-sdk/util/generic/utility.h>

/**
 * Proxy input stream that can read a limited number of characters from a slave
 * stream.
 *
 * This can be useful for breaking up the slave stream into small chunks and
 * treat these as separate streams.
 */
class TLengthLimitedInput: public IInputStream {
public:
    inline TLengthLimitedInput(IInputStream* slave, ui64 length) noexcept
        : Slave_(slave)
        , Length_(length)
    {
    }

    ~TLengthLimitedInput() override = default;

    inline ui64 Left() const noexcept {
        return Length_;
    }

private:
    size_t DoRead(void* buf, size_t len) override;
    size_t DoSkip(size_t len) override;

private:
    IInputStream* Slave_;
    ui64 Length_;
};
