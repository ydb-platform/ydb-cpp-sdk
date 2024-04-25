#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/fwd.h>
=======
#include <src/util/generic/fwd.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/fwd.h>
>>>>>>> origin/main

// NOTE: This function has rare sporadic failures (throws exceptions) on FreeBSD. See REVIEW:54297
const std::string& GetExecPath();

/**
 * Get openable path to the binary currently being executed.
 *
 * The path does not match the original binary location, but stays openable even
 * if the binary was moved or removed.
 *
 * On UNIX variants, utilizes the /proc FS. On Windows, equivalent to
 * GetExecPath.
 */
const std::string& GetPersistentExecPath();
