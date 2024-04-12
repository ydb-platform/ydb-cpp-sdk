#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/client/iam_private/iam.h
#include <ydb-cpp-sdk/client/iam/common/iam.h>
========
#include <src/client/iam/common/iam.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/iam_private/iam.h

namespace NYdb {

/// Acquire an IAM token using a JSON Web Token (JWT) file name.
TCredentialsProviderFactoryPtr CreateIamJwtFileCredentialsProviderFactoryPrivate(const TIamJwtFilename& params);

/// Acquire an IAM token using JSON Web Token (JWT) contents.
TCredentialsProviderFactoryPtr CreateIamJwtParamsCredentialsProviderFactoryPrivate(const TIamJwtContent& param);

} // namespace NYdb
