#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/client/types/credentials/credentials.h
#include <ydb-cpp-sdk/util/system/compiler.h>
========
#include <src/util/system/compiler.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_types/credentials/credentials.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_types/credentials/credentials.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <memory>
#include <string>

namespace NYdb {

class ICredentialsProvider {
public:
    virtual ~ICredentialsProvider() = default;
    virtual std::string GetAuthInfo() const = 0;
    virtual bool IsValid() const = 0;
};

using TCredentialsProviderPtr = std::shared_ptr<ICredentialsProvider>;

class ICoreFacility;
class ICredentialsProviderFactory {
public:
    virtual ~ICredentialsProviderFactory() = default;
    virtual TCredentialsProviderPtr CreateProvider() const = 0;
    // !!!Experimental!!!
    virtual TCredentialsProviderPtr CreateProvider(std::weak_ptr<ICoreFacility> facility) const {
        Y_UNUSED(facility);
        return CreateProvider();
    }
    virtual std::string GetClientIdentity() const;
};

using TCredentialsProviderFactoryPtr = std::shared_ptr<ICredentialsProviderFactory>;

std::shared_ptr<ICredentialsProviderFactory> CreateInsecureCredentialsProviderFactory();
std::shared_ptr<ICredentialsProviderFactory> CreateOAuthCredentialsProviderFactory(const std::string& token);

struct TLoginCredentialsParams {
    std::string User;
    std::string Password;
};

std::shared_ptr<ICredentialsProviderFactory> CreateLoginCredentialsProviderFactory(TLoginCredentialsParams params);

} // namespace NYdb
