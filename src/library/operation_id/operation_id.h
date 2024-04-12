#pragma once

#include <src/library/operation_id/protos/operation_id.pb.h>

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/operation_id/operation_id.h
========
#include <src/util/generic/hash.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/operation_id/operation_id.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/operation_id/operation_id.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
#include <string>
#include <vector>

namespace NKikimr {
namespace NOperationId {

class TOperationId {
public:
    TOperationId();
    explicit TOperationId(const std::string& string, bool allowEmpty = false);
    const std::vector<const std::string*>& GetValue(const std::string& key) const;
    std::string GetSubKind() const;
    const Ydb::TOperationId& GetProto() const;

private:
    Ydb::TOperationId Proto_;
    std::unordered_map<std::string, std::vector<const std::string*>> Index_;
};

std::string ProtoToString(const Ydb::TOperationId& proto);
void AddOptionalValue(Ydb::TOperationId& proto, const std::string& key, const std::string& value);
void AddOptionalValue(Ydb::TOperationId& proto, const std::string& key, const char* value, size_t size);
Ydb::TOperationId::EKind ParseKind(const std::string_view value);

std::string FormatPreparedQueryIdCompat(const std::string& str);
bool DecodePreparedQueryIdCompat(const std::string& in, std::string& out);

} // namespace NOperationId
} // namespace NKikimr
