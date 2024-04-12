#pragma once

#include <ydb-cpp-sdk/library/yql/public/issue/yql_issue.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/ylimits.h>
=======
#include <src/util/generic/ylimits.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NYql {

namespace NIssue {
namespace NProto {
class IssueMessage;
}
}

template<typename TIssueMessage>
TIssue IssueFromMessage(const TIssueMessage& issueMessage);
template<typename TIssueMessage>
void IssuesFromMessage(const ::google::protobuf::RepeatedPtrField<TIssueMessage>& message, TIssues& issues);

template<typename TIssueMessage>
std::string IssuesFromMessageAsString(const ::google::protobuf::RepeatedPtrField<TIssueMessage>& message) {
    TIssues issues;
    IssuesFromMessage(message, issues);
    return issues.ToOneLineString();
}

NIssue::NProto::IssueMessage IssueToMessage(const TIssue& topIssue);

template<typename TIssueMessage>
void IssueToMessage(const TIssue& topIssue, TIssueMessage* message);
template<typename TIssueMessage>
void IssuesToMessage(const TIssues& issues, ::google::protobuf::RepeatedPtrField<TIssueMessage>* message);

std::string IssueToBinaryMessage(const TIssue& issue);
TIssue IssueFromBinaryMessage(const std::string& binaryMessage);

}
