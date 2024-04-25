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

#include <src/api/client/yc_public/iam/iam_token_service.pb.h>
#include <src/api/client/yc_public/iam/iam_token_service.grpc.pb.h>

using namespace yandex::cloud::iam::v1;

namespace NYdb {

TCredentialsProviderFactoryPtr CreateIamJwtFileCredentialsProviderFactory(const TIamJwtFilename& params) {
    TIamJwtParams jwtParams = { params, ReadJwtKeyFile(params.JwtFilename) };
    return std::make_shared<TIamJwtCredentialsProviderFactory<CreateIamTokenRequest,
                                                              CreateIamTokenResponse,
                                                              IamTokenService>>(std::move(jwtParams));
}

TCredentialsProviderFactoryPtr CreateIamJwtParamsCredentialsProviderFactory(const TIamJwtContent& params) {
    TIamJwtParams jwtParams = { params, ParseJwtParams(params.JwtContent) };
    return std::make_shared<TIamJwtCredentialsProviderFactory<CreateIamTokenRequest,
                                                              CreateIamTokenResponse,
                                                              IamTokenService>>(std::move(jwtParams));
}

TCredentialsProviderFactoryPtr CreateIamOAuthCredentialsProviderFactory(const TIamOAuth& params) {
    return std::make_shared<TIamOAuthCredentialsProviderFactory<CreateIamTokenRequest,
                                                                CreateIamTokenResponse,
                                                                IamTokenService>>(params);
}

}
