#pragma once
#include <optional>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <deque>
#include <util/generic/string.h>
#include <ydb/library/login/protos/login.pb.h>

namespace NLogin {

using NLoginProto::ESidType;

class TLoginProvider {
public:
    static constexpr size_t MAX_SERVER_KEYS = 1000;
    static constexpr size_t MAX_CLIENT_KEYS = 100000;
    static constexpr auto KEYS_ROTATION_PERIOD = std::chrono::hours(6);
    static constexpr auto KEY_EXPIRE_TIME = std::chrono::hours(24);

    static constexpr size_t SALT_SIZE = 16;
    static constexpr size_t HASH_SIZE = 32;

    static constexpr const char* GROUPS_CLAIM_NAME = "https://ydb.tech/groups";
    static constexpr const char* EXTERNAL_AUTH_CLAIM_NAME = "external_authentication";
    static constexpr auto MAX_TOKEN_EXPIRE_TIME = std::chrono::hours(12);

    struct TBasicRequest {};

    struct TBasicResponse {
        std::string Error;
        std::string Warning;
        std::string Notice;
    };

    struct TLoginUserRequest : TBasicRequest {
        struct TOptions {
            bool WithUserGroups = false;
            std::chrono::system_clock::duration ExpiresAfter = std::chrono::system_clock::duration::zero();
        };

        std::string User;
        std::string Password;
        TOptions Options;
        std::string ExternalAuth;
    };

    struct TLoginUserResponse : TBasicResponse {
        std::string Token;
    };

    struct TValidateTokenRequest : TBasicRequest {
        std::string Token;
    };

    struct TValidateTokenResponse : TBasicResponse {
        bool TokenUnrecognized = false;
        bool ErrorRetryable = false;
        std::string User;
        std::optional<std::vector<std::string>> Groups;
        std::chrono::system_clock::time_point ExpiresAt;
        std::string ExternalAuth;
    };

    struct TCreateUserRequest : TBasicRequest {
        std::string User;
        std::string Password;
    };

    struct TModifyUserRequest : TBasicRequest {
        std::string User;
        std::string Password;
    };

    struct TRemoveUserRequest : TBasicRequest {
        std::string User;
        bool MissingOk;
    };

    struct TRemoveUserResponse : TBasicResponse {
        std::vector<std::string> TouchedGroups;
    };

    struct TCreateGroupRequest : TBasicRequest {
        struct TOptions {
            bool CheckName = true;
        };

        std::string Group;
        TOptions Options;
    };

    struct TAddGroupMembershipRequest : TBasicRequest {
        std::string Group;
        std::string Member;
    };

    struct TRemoveGroupMembershipRequest : TBasicRequest {
        std::string Group;
        std::string Member;
    };

    struct TRenameGroupRequest : TBasicRequest {
        struct TOptions {
            bool CheckName = true;
        };

        std::string Group;
        std::string NewName;
        TOptions Options;
    };

    struct TRenameGroupResponse : TBasicResponse {
        std::vector<std::string> TouchedGroups;
    };

    struct TRemoveGroupRequest : TBasicRequest {
        std::string Group;
        bool MissingOk;
    };

    struct TRemoveGroupResponse : TBasicResponse {
        std::vector<std::string> TouchedGroups;
    };

    struct TKeyRecord {
        ui64 KeyId;
        std::string PublicKey;
        std::string PrivateKey;
        std::chrono::system_clock::time_point ExpiresAt;
    };

    std::deque<TKeyRecord> Keys; // it's always ordered by KeyId
    std::chrono::time_point<std::chrono::system_clock> KeysRotationTime;

    struct TSidRecord {
        ESidType::SidType Type = ESidType::UNKNOWN;
        std::string Name;
        std::string Hash;
        std::unordered_set<std::string> Members;
    };

    // our current audience (database name)
    std::string Audience;

    // all users and theirs hashs
    std::unordered_map<std::string, TSidRecord> Sids;

    // index for fast traversal
    std::unordered_map<std::string, std::unordered_set<std::string>> ChildToParentIndex;

    // servers duty to call this method periodically (and publish PublicKeys after that)
    const TKeyRecord* FindKey(ui64 keyId);
    void RotateKeys();
    void RotateKeys(std::vector<ui64>& keysExpired, std::vector<ui64>& keysAdded);
    bool IsItTimeToRotateKeys() const;
    NLoginProto::TSecurityState GetSecurityState() const;
    void UpdateSecurityState(const NLoginProto::TSecurityState& state);

    TLoginUserResponse LoginUser(const TLoginUserRequest& request);
    TValidateTokenResponse ValidateToken(const TValidateTokenRequest& request);

    TBasicResponse CreateUser(const TCreateUserRequest& request);
    TBasicResponse ModifyUser(const TModifyUserRequest& request);
    TRemoveUserResponse RemoveUser(const TRemoveUserRequest& request);
    bool CheckUserExists(const std::string& name);

    TBasicResponse CreateGroup(const TCreateGroupRequest& request);
    TBasicResponse AddGroupMembership(const TAddGroupMembershipRequest& request);
    TBasicResponse RemoveGroupMembership(const TRemoveGroupMembershipRequest& request);
    TRenameGroupResponse RenameGroup(const TRenameGroupRequest& request);
    TRemoveGroupResponse RemoveGroup(const TRemoveGroupRequest& request);

    TLoginProvider();
    ~TLoginProvider();

    std::vector<std::string> GetGroupsMembership(const std::string& member);
    static std::string GetTokenAudience(const std::string& token);
    static std::chrono::system_clock::time_point GetTokenExpiresAt(const std::string& token);

private:
    std::deque<TKeyRecord>::iterator FindKeyIterator(ui64 keyId);
    bool CheckSubjectExists(const std::string& name, const ESidType::SidType& type);
    static bool CheckAllowedName(const std::string& name);

    struct TImpl;
    THolder<TImpl> Impl;
};

}
