#pragma once

#include <string>
#include <string_view>

namespace NUtils {

    /**
      * Search the environment list provided by the host environment for associated variable.
      *
      * @param key   String identifying the name of the environmental variable to look for
      * @param def   String that returns if environmental variable not found by key
      *
      * @return      String that is associated with the matched environment variable or empty string
      *              if such variable is missing.
      *
      * @note        Use it only in pair with `SetEnv` as there may be inconsistency in their
      *              behaviour otherwise.
      * @note        Calls to `GetEnv` and `SetEnv` from different threads must be synchronized.
      * @see         SetEnv
      */
    std::string GetEnv(std::string_view key, std::string_view def = "");

    /**
      * Add or change environment variable provided by the host environment.
      *
      * @param key         String identifying the name of the environment variable to set or change
      * @param new_value   Value to assign
      * @note              Use it only in pair with `GetEnv` as there may be inconsistency in their
      *                    behaviour otherwise.
      * @note              Calls to `GetEnv` and `SetEnv` from different threads must be synchronized.
      * @see               GetEnv
      */
    void SetEnv(std::string_view key, std::string_view new_value);

} // namespace NUtils