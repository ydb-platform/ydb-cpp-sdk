#pragma once

#include <ostream>

/**
 * @returns                             Standard debug stream.
 * @see Cdbg
 */
std::ostream& StdDbgStream() noexcept;

/**
 * Standard debug stream.
 *
 * Behavior of this stream is controlled via `DBGOUT` environment variable.
 * If this variable is set, then this stream is redirected into `stderr`,
 * otherwise whatever is written into it is simply ignored.
 */
#define Cdbg (StdDbgStream())

/** @} */
