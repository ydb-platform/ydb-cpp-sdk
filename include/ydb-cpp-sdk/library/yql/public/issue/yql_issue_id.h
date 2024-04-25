#pragma once

#include <src/library/yql/public/issue/protos/issue_severity.pb.h>

<<<<<<< HEAD
<<<<<<<< HEAD:include/ydb-cpp-sdk/library/yql/public/issue/yql_issue_id.h
#include <ydb-cpp-sdk/library/resource/resource.h>
#include <ydb-cpp-sdk/library/string_utils/misc/misc.h>
========
#include <src/library/resource/resource.h>
#include <src/library/string_utils/misc/misc.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/yql/public/issue/yql_issue_id.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/yql/public/issue/yql_issue_id.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/library/resource/resource.h>
#include <ydb-cpp-sdk/library/string_utils/misc/misc.h>
>>>>>>> origin/main

#include <google/protobuf/descriptor.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/message.h>

<<<<<<< HEAD
<<<<<<<< HEAD:include/ydb-cpp-sdk/library/yql/public/issue/yql_issue_id.h
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
#include <ydb-cpp-sdk/util/string/subst.h>
========
#include <src/util/generic/hash.h>
#include <src/util/generic/singleton.h>
#include <src/util/generic/yexception.h>
#include <src/util/string/subst.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/yql/public/issue/yql_issue_id.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/yql/public/issue/yql_issue_id.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
#include <ydb-cpp-sdk/util/string/subst.h>
>>>>>>> origin/main

#ifdef _win_
#ifdef GetMessage
#undef GetMessage
#endif
#endif

namespace NYql {

using TIssueCode = ui32;
using ESeverity = NYql::TSeverityIds::ESeverityId;
const TIssueCode DEFAULT_ERROR = 0;
const TIssueCode UNEXPECTED_ERROR = 1;

inline std::string SeverityToString(ESeverity severity)
{
    auto ret = NYql::TSeverityIds::ESeverityId_Name(severity);
    return ret.empty() ? "Unknown" : NUtils::ToTitle(ret.substr(2)); //remove prefix "S_"
}

template <typename T>
inline std::string IssueCodeToString(TIssueCode id) {
    auto ret = T::EIssueCode_Name(static_cast<typename T::EIssueCode>(id));
    if (!ret.empty()) {
        SubstGlobal(ret, '_', ' ');
        return to_title(ret);
    } else {
        return "Unknown";
    }
}

template<typename TProto, const char* ResourceName>
class TIssueId {
    TProto ProtoIssues_;
    std::unordered_map<TIssueCode, NYql::TSeverityIds::ESeverityId> IssuesMap_;
    std::unordered_map<TIssueCode, std::string> IssuesFormatMap_;

    const google::protobuf::Descriptor* GetProtoDescriptor() const {
        auto ret = ProtoIssues_.GetDescriptor();
        Y_ENSURE(ret != nullptr, "Bad proto file");
        return ret;
    }

    bool CheckSeverityNameFormat(const std::string& name) const {
        if (name.size() > 2 && name.substr(0,2) == "S_") {
            return true;
        }
        return false;
    }

public:
    ESeverity GetSeverity(TIssueCode id) const {
        auto it = IssuesMap_.find(id);
        Y_ENSURE(it != IssuesMap_.end(), "Unknown issue id: "
            << id << "(" << IssueCodeToString<TProto>(id) << ")");
        return it->second;
    }

    std::string GetMessage(TIssueCode id) const {
        auto it = IssuesFormatMap_.find(id);
        Y_ENSURE(it != IssuesFormatMap_.end(), "Unknown issue id: "
            << id << "(" << IssueCodeToString<TProto>(id) << ")");
        return it->second;
    }

    TIssueId() {
        auto configData = NResource::Find(std::string_view(ResourceName));
        if (!::google::protobuf::TextFormat::ParseFromString(configData, &ProtoIssues_)) {
            Y_ENSURE(false, "Bad format of protobuf data file, resource: " << ResourceName);
        }

        auto sDesc = TSeverityIds::ESeverityId_descriptor();
        for (int i = 0; i < sDesc->value_count(); i++) {
            const auto& name = sDesc->value(i)->name();
            Y_ENSURE(CheckSeverityNameFormat(name),
                "Wrong severity name: " << name << ". Severity must starts with \"S_\" prefix");
        }

        for (const auto& x : ProtoIssues_.ids()) {
            auto rv = IssuesMap_.insert(std::make_pair(x.code(), x.severity()));
            Y_ENSURE(rv.second, "Duplicate issue code found, code: "
                << static_cast<int>(x.code())
                << "(" << IssueCodeToString<TProto>(x.code()) <<")");
        }

        // Check all IssueCodes have mapping to severity
        auto eDesc = TProto::EIssueCode_descriptor();
        for (int i = 0; i < eDesc->value_count(); i++) {
            auto it = IssuesMap_.find(eDesc->value(i)->number());
            Y_ENSURE(it != IssuesMap_.end(), "IssueCode: "
                << eDesc->value(i)->name()
                << " is not found in protobuf data file");
        }

        for (const auto& x : ProtoIssues_.ids()) {
            auto rv = IssuesFormatMap_.insert(std::make_pair(x.code(), x.format()));
            Y_ENSURE(rv.second, "Duplicate issue code found, code: "
                << static_cast<int>(x.code())
                << "(" << IssueCodeToString<TProto>(x.code()) <<")");
        }
    }
};

template<typename TProto, const char* ResourceName>
inline ESeverity GetSeverity(TIssueCode id) {
    return Singleton<TIssueId<TProto, ResourceName>>()->GetSeverity(id);
}

template<typename TProto, const char* ResourceName>
inline std::string GetMessage(TIssueCode id) {
    return Singleton<TIssueId<TProto, ResourceName>>()->GetMessage(id);
}

}
