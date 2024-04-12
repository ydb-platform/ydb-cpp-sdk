#include <ydb-cpp-sdk/util/stream/zerocopy_output.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/utility.h>
=======
#include <src/util/generic/utility.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

void IZeroCopyOutput::DoWrite(const void* buf, size_t len) {
    void* ptr = nullptr;
    size_t writtenBytes = 0;
    while (writtenBytes < len) {
        size_t bufferSize = DoNext(&ptr);
        Y_ASSERT(ptr && bufferSize > 0);
        size_t toWrite = Min(bufferSize, len - writtenBytes);
        memcpy(ptr, static_cast<const char*>(buf) + writtenBytes, toWrite);
        writtenBytes += toWrite;
        if (toWrite < bufferSize) {
            DoUndo(bufferSize - toWrite);
        }
    }
}
