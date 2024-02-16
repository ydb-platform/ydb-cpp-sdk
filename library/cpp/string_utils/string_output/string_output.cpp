#include "string_output.h"

#include <util/system/yassert.h>

namespace NUtils {

inline void TStringOutput::Reserve(size_t size) {
    S_->reserve(S_->size() + size);
}

inline void TStringOutput::Undo(size_t len) {
    Y_ABORT_UNLESS(len <= S_->size(), "trying to undo more bytes than actually written");
    S_->resize(S_->size() - len);
}

void TStringOutput::DoWrite(const void* buf, size_t len) {
    S_->append((const char*)buf, len);
}

void TStringOutput::DoWriteC(char c) {
    S_->push_back(c);
}

}