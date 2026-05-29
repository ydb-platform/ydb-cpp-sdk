#include "slo_text_utils.h"

#include <util/folder/path.h>
#include <util/random/random.h>

#include <string_view>

std::string SloJoinPath(const std::string& prefix, const std::string& path) {
    if (prefix.empty()) {
        return path;
    }
    TPathSplitUnix prefixPathSplit(prefix);
    prefixPathSplit.AppendComponent(path);
    return prefixPathSplit.Reconstruct();
}

std::string SloGetDatabaseFromConnectionString(const std::string& connectionString) {
    constexpr std::string_view databaseFlag = "/?database=";
    size_t pathIndex = connectionString.find(databaseFlag);
    if (pathIndex != std::string::npos) {
        return connectionString.substr(pathIndex + databaseFlag.size());
    }
    return {};
}

std::string SloGenerateRandomString(std::uint32_t minLength, std::uint32_t maxLength) {
    std::uint32_t length = minLength + RandomNumber<std::uint32_t>() % (maxLength - minLength);
    static const char* symbols = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result.push_back(symbols[RandomNumber<std::uint8_t>(61)]);
    }
    return result;
}

std::string SloYdbStatusToString(NYdb::EStatus status) {
    switch (status) {
        case NYdb::EStatus::SUCCESS:
            return "SUCCESS";
        case NYdb::EStatus::BAD_REQUEST:
            return "BAD_REQUEST";
        case NYdb::EStatus::UNAUTHORIZED:
            return "UNAUTHORIZED";
        case NYdb::EStatus::INTERNAL_ERROR:
            return "INTERNAL_ERROR";
        case NYdb::EStatus::ABORTED:
            return "ABORTED";
        case NYdb::EStatus::UNAVAILABLE:
            return "UNAVAILABLE";
        case NYdb::EStatus::OVERLOADED:
            return "OVERLOADED";
        case NYdb::EStatus::SCHEME_ERROR:
            return "SCHEME_ERROR";
        case NYdb::EStatus::GENERIC_ERROR:
            return "GENERIC_ERROR";
        case NYdb::EStatus::TIMEOUT:
            return "TIMEOUT";
        case NYdb::EStatus::BAD_SESSION:
            return "BAD_SESSION";
        case NYdb::EStatus::PRECONDITION_FAILED:
            return "PRECONDITION_FAILED";
        case NYdb::EStatus::ALREADY_EXISTS:
            return "ALREADY_EXISTS";
        case NYdb::EStatus::NOT_FOUND:
            return "NOT_FOUND";
        case NYdb::EStatus::SESSION_EXPIRED:
            return "SESSION_EXPIRED";
        case NYdb::EStatus::CANCELLED:
            return "CANCELLED";
        case NYdb::EStatus::UNDETERMINED:
            return "UNDETERMINED";
        case NYdb::EStatus::UNSUPPORTED:
            return "UNSUPPORTED";
        case NYdb::EStatus::SESSION_BUSY:
            return "SESSION_BUSY";
        case NYdb::EStatus::EXTERNAL_ERROR:
            return "EXTERNAL_ERROR";
        case NYdb::EStatus::STATUS_UNDEFINED:
            return "STATUS_UNDEFINED";
        case NYdb::EStatus::TRANSPORT_UNAVAILABLE:
            return "TRANSPORT_UNAVAILABLE";
        case NYdb::EStatus::CLIENT_RESOURCE_EXHAUSTED:
            return "CLIENT_RESOURCE_EXHAUSTED";
        case NYdb::EStatus::CLIENT_DEADLINE_EXCEEDED:
            return "CLIENT_DEADLINE_EXCEEDED";
        case NYdb::EStatus::CLIENT_INTERNAL_ERROR:
            return "CLIENT_INTERNAL_ERROR";
        case NYdb::EStatus::CLIENT_CANCELLED:
            return "CLIENT_CANCELLED";
        case NYdb::EStatus::CLIENT_UNAUTHENTICATED:
            return "CLIENT_UNAUTHENTICATED";
        case NYdb::EStatus::CLIENT_CALL_UNIMPLEMENTED:
            return "CLIENT_CALL_UNIMPLEMENTED";
        case NYdb::EStatus::CLIENT_OUT_OF_RANGE:
            return "CLIENT_OUT_OF_RANGE";
        case NYdb::EStatus::CLIENT_DISCOVERY_FAILED:
            return "CLIENT_DISCOVERY_FAILED";
        case NYdb::EStatus::CLIENT_LIMITS_REACHED:
            return "CLIENT_LIMITS_REACHED";
    }
}

NYdb::EStatus SloYdbStatusFromString(const std::string& label) {
    if (label == "SUCCESS") {
        return NYdb::EStatus::SUCCESS;
    }
    if (label == "BAD_REQUEST") {
        return NYdb::EStatus::BAD_REQUEST;
    }
    if (label == "UNAUTHORIZED") {
        return NYdb::EStatus::UNAUTHORIZED;
    }
    if (label == "INTERNAL_ERROR") {
        return NYdb::EStatus::INTERNAL_ERROR;
    }
    if (label == "ABORTED") {
        return NYdb::EStatus::ABORTED;
    }
    if (label == "UNAVAILABLE") {
        return NYdb::EStatus::UNAVAILABLE;
    }
    if (label == "OVERLOADED") {
        return NYdb::EStatus::OVERLOADED;
    }
    if (label == "SCHEME_ERROR") {
        return NYdb::EStatus::SCHEME_ERROR;
    }
    if (label == "GENERIC_ERROR" || label == "UNKNOWN_ERROR") {
        return NYdb::EStatus::GENERIC_ERROR;
    }
    if (label == "TIMEOUT" || label == "APPLICATION_TIMEOUT") {
        return NYdb::EStatus::TIMEOUT;
    }
    if (label == "BAD_SESSION") {
        return NYdb::EStatus::BAD_SESSION;
    }
    if (label == "PRECONDITION_FAILED") {
        return NYdb::EStatus::PRECONDITION_FAILED;
    }
    if (label == "ALREADY_EXISTS") {
        return NYdb::EStatus::ALREADY_EXISTS;
    }
    if (label == "NOT_FOUND") {
        return NYdb::EStatus::NOT_FOUND;
    }
    if (label == "SESSION_EXPIRED") {
        return NYdb::EStatus::SESSION_EXPIRED;
    }
    if (label == "CANCELLED") {
        return NYdb::EStatus::CANCELLED;
    }
    if (label == "UNDETERMINED") {
        return NYdb::EStatus::UNDETERMINED;
    }
    if (label == "UNSUPPORTED") {
        return NYdb::EStatus::UNSUPPORTED;
    }
    if (label == "SESSION_BUSY") {
        return NYdb::EStatus::SESSION_BUSY;
    }
    if (label == "EXTERNAL_ERROR") {
        return NYdb::EStatus::EXTERNAL_ERROR;
    }
    if (label == "STATUS_UNDEFINED") {
        return NYdb::EStatus::STATUS_UNDEFINED;
    }
    if (label == "TRANSPORT_UNAVAILABLE") {
        return NYdb::EStatus::TRANSPORT_UNAVAILABLE;
    }
    if (label == "CLIENT_RESOURCE_EXHAUSTED") {
        return NYdb::EStatus::CLIENT_RESOURCE_EXHAUSTED;
    }
    if (label == "CLIENT_DEADLINE_EXCEEDED") {
        return NYdb::EStatus::CLIENT_DEADLINE_EXCEEDED;
    }
    if (label == "CLIENT_INTERNAL_ERROR") {
        return NYdb::EStatus::CLIENT_INTERNAL_ERROR;
    }
    if (label == "CLIENT_CANCELLED") {
        return NYdb::EStatus::CLIENT_CANCELLED;
    }
    if (label == "CLIENT_UNAUTHENTICATED") {
        return NYdb::EStatus::CLIENT_UNAUTHENTICATED;
    }
    if (label == "CLIENT_CALL_UNIMPLEMENTED") {
        return NYdb::EStatus::CLIENT_CALL_UNIMPLEMENTED;
    }
    if (label == "CLIENT_OUT_OF_RANGE") {
        return NYdb::EStatus::CLIENT_OUT_OF_RANGE;
    }
    if (label == "CLIENT_DISCOVERY_FAILED") {
        return NYdb::EStatus::CLIENT_DISCOVERY_FAILED;
    }
    if (label == "CLIENT_LIMITS_REACHED") {
        return NYdb::EStatus::CLIENT_LIMITS_REACHED;
    }
    return NYdb::EStatus::GENERIC_ERROR;
}
