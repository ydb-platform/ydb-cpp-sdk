#pragma once

<<<<<<<< HEAD:src/client/table/impl/data_query.h
#include <ydb-cpp-sdk/client/table/table.h>
========
#include <src/client/ydb_table/table.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_table/impl/data_query.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_table/impl/data_query.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
#include <src/api/protos/ydb_table.pb.h>

namespace NYdb {
namespace NTable {

std::string EncodeQuery(const std::string& text, bool reversible);

////////////////////////////////////////////////////////////////////////////////

class TDataQuery::TImpl {
    friend class TDataQuery;

public:
    TImpl(const TSession& session, const std::string& text, bool keepText, const std::string& id, bool allowMigration);

    TImpl(const TSession& session, const std::string& text, bool keepText, const std::string& id, bool allowMigration,
        const ::google::protobuf::Map<std::string, Ydb::Type>& types);

    const std::string& GetId() const;
    const ::google::protobuf::Map<std::string, Ydb::Type>& GetParameterTypes() const;
    const std::string& GetTextHash() const;
    const std::optional<std::string>& GetText() const;

private:
    NTable::TSession Session_;
    std::string Id_;
    ::google::protobuf::Map<std::string, Ydb::Type> ParameterTypes_;
    std::string TextHash_;
    std::optional<std::string> Text_;
};

} // namespace NTable
} // namespace NYdb
