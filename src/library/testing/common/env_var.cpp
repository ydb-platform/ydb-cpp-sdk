#include "env_var.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/yexception.h>

#include <ydb-cpp-sdk/util/system/platform.h>
=======
#include <src/util/generic/yexception.h>

#include <src/util/system/platform.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

#if defined(_win_)
    #include <src/util/system/wininit.h>
#else
    #include <cerrno>
#endif

#include <cstdlib>

namespace NUtils {

    std::string GetEnv(std::string_view key, std::string_view def) {
        const char* str_ptr = std::getenv(key.data());
        return std::string(str_ptr == nullptr ? def : str_ptr);
    }

    void SetEnv(std::string_view key, std::string_view new_value) {
        bool is_ok = false;
        int error_code = 0;
#if defined(_win_)
        is_ok = SetEnvironmentVariable(key.data(), new_value.data()) == 0;
        if (!is_ok) {
            error_code = GetLastError();
        }
#else
        is_ok = setenv(key.data(), new_value.data(), /* overwrite */ true) == 0;
        if (!is_ok) {
            error_code = errno;
        }
#endif
        Y_ENSURE_EX(
            is_ok, 
            TSystemError() << "failed to SetEnv with error-code " << error_code
        );
    }

} // namespace NUtils