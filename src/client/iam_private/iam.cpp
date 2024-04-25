#include <ydb-cpp-sdk/client/iam_private/iam.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/client/iam/common/iam.h>
=======
#include <src/client/iam/common/iam.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/client/iam/common/iam.h>
>>>>>>> origin/main

#include <src/api/client/yc_private/iam/iam_token_service.pb.h>
#include <src/api/client/yc_private/iam/iam_token_service.grpc.pb.h>

namespace NYdb {

TCredentialsProviderFactoryPtr CreateIamJwtCredentialsProviderFactoryImplPrivate(TIamJwtParams&& jwtParams) {
    return std::make_shared<TIamJwtCredentialsProviderFactory<
                    yandex::cloud::priv::iam::v1::CreateIamTokenRequest,
                    yandex::cloud::priv::iam::v1::CreateIamTokenResponse,
                    yandex::cloud::priv::iam::v1::IamTokenService
                >>(std::move(jwtParams));
}

TCredentialsProviderFactoryPtr CreateIamJwtFileCredentialsProviderFactoryPrivate(const TIamJwtFilename& params) {
    TIamJwtParams jwtParams = { params, ReadJwtKeyFile(params.JwtFilename) };
    return CreateIamJwtCredentialsProviderFactoryImplPrivate(std::move(jwtParams));
}

TCredentialsProviderFactoryPtr CreateIamJwtParamsCredentialsProviderFactoryPrivate(const TIamJwtContent& params) {
    TIamJwtParams jwtParams = { params, ParseJwtParams(params.JwtContent) };
    return CreateIamJwtCredentialsProviderFactoryImplPrivate(std::move(jwtParams));
}

}
