#include <ydb-cpp-sdk/client/proto/accessor.h>

#include <ydb-cpp-sdk/client/import/import.h>

namespace NYdb::inline V3 {

const Ydb::Import::ListObjectsInS3ExportResult& TProtoAccessor::GetProto(const NYdb::NImport::TListObjectsInS3ExportResult& result) {
    return result.GetProto();
}

}
