#include <src/api/protos/ydb_issue_message.pb.h>
#include <src/library/yql_common/issue/protos/issue_message.pb.h>

#include <ydb-cpp-sdk/type_switcher.h>

#include <google/protobuf/text_format.h>

#include <util/stream/output.h>

Y_DECLARE_OUT_SPEC(, Ydb::Issue::IssueMessage, stream, value) {
    google::protobuf::TextFormat::Printer printer;
    printer.SetSingleLineMode(true);
    printer.SetUseUtf8StringEscaping(true);

    NYdb::TStringType str;
    printer.PrintToString(value, &str);

    // Copied from text_format.h
    // Single line mode currently might have an extra space at the end.
    if (str.size() > 0 && str[str.size() - 1] == ' ') {
        str.resize(str.size() - 1);
    }

    stream << "{ " << str << " }";
}

Y_DECLARE_OUT_SPEC(, NYql::NIssue::NProto::IssueMessage, stream, value) {
    google::protobuf::TextFormat::Printer printer;
    printer.SetSingleLineMode(true);
    printer.SetUseUtf8StringEscaping(true);

    NYdb::TStringType str;
    printer.PrintToString(value, &str);

    // Copied from text_format.h
    // Single line mode currently might have an extra space at the end.
    if (str.size() > 0 && str[str.size() - 1] == ' ') {
        str.resize(str.size() - 1);
    }

    stream << "{ " << str << " }";
}
