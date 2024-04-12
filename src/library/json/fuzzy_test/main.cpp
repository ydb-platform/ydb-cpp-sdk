<<<<<<< HEAD
#include <ydb-cpp-sdk/library/json/json_reader.h>

#include <ydb-cpp-sdk/util/random/random.h>
#include <ydb-cpp-sdk/util/stream/str.h>
=======
#include <src/library/json/json_reader.h>

#include <src/util/random/random.h>
#include <src/util/stream/str.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

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
