#pragma once

#include <string>

#include <util/system/env.h>

#include <utility>
#include <vector>

namespace NTesting {
    // @brief Assigns new values to the given environment variables and restores old values upon destruction.
    // @note if there was no env variable with given name, it will be set to empty string upon destruction IGNIETFERRO-1486
    struct TScopedEnvironment {
        TScopedEnvironment(const std::string& name, const std::string& value)
            : PreviousState{1, {name, ::GetEnv(name)}}
        {
            ::SetEnv(name, value);
        }

        TScopedEnvironment(const std::vector<std::pair<std::string, std::string>>& vars)
            : PreviousState{}
        {
            PreviousState.reserve(vars.size());
            for (const auto& [k, v] : vars) {
                PreviousState.emplace_back(k, ::GetEnv(k));
                ::SetEnv(k, v);
            }
        }

        ~TScopedEnvironment() {
            for (const auto& [k, v] : PreviousState) {
                ::SetEnv(k, v);
            }
        }

        TScopedEnvironment(const TScopedEnvironment&) = delete;
        TScopedEnvironment& operator=(const TScopedEnvironment&) = delete;
    private:
        std::vector<std::pair<std::string, std::string>> PreviousState;
    };
}
