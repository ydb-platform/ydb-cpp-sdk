#pragma once

#include <string>

#include <array>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

struct TThreadName
{
    TThreadName() = default;
    TThreadName(const std::string& name);

    std::string_view ToStringBuf() const;

    static constexpr int BufferCapacity = 16; // including zero terminator
    std::array<char, BufferCapacity> Buffer{}; // zero-terminated
    int Length = 0; // not including zero terminator
};

TThreadName GetCurrentThreadName();

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
