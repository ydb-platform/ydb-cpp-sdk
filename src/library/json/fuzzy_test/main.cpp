#include <ydb-cpp-sdk/library/json/json_reader.h>

#include <ydb-cpp-sdk/util/random/random.h>
#include <src/util/stream/str.h>

extern "C" int LLVMFuzzerTestOneInput(const ui8* data, size_t size) {
    const auto json = std::string((const char*)data, size);

    try {
        NJson::TJsonValue value;
        NJson::ReadJsonFastTree(json, &value, true);
    } catch (...) {
        //std::cout << json << " -> " << CurrentExceptionMessage() << std::endl;
    }

    try {
        NJson::TJsonCallbacks cb;
        NJson::ReadJsonFast(json, &cb);
    } catch (...) {
        //std::cout << json << " -> " << CurrentExceptionMessage() << std::endl;
    }

    try {
        NJson::ValidateJson(json);
    } catch (...) {
        //std::cout << json << " -> " << CurrentExceptionMessage() << std::endl;
    }

    return 0;
}
