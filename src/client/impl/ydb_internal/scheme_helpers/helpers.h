#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/client/scheme/scheme.h>
=======
#include <src/client/ydb_scheme/scheme.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/client/scheme/scheme.h>
>>>>>>> origin/main
#include <vector>

namespace NYdb {

template<typename TFrom>
inline void PermissionToSchemeEntry(const TFrom& from, std::vector<NScheme::TPermissions>* to) {
    for (const auto& effPerm : from) {
        to->push_back(NScheme::TPermissions{effPerm.subject(), std::vector<std::string>()});
        for (const auto& permName : effPerm.permission_names()) {
            to->back().PermissionNames.push_back(permName);
        }
    }
}

} // namespace NYdb
