#include "stream.h"
#include <ydb-cpp-sdk/library/logger/record.h>

#include <src/util/string/builder.h>

#include <src/util/stream/output.h>

#include <ostream>


TStreamLogBackend::TStreamLogBackend(std::ostream* slave)
    : Slave_(slave)
{
}

TStreamLogBackend::~TStreamLogBackend() {
}

void TStreamLogBackend::WriteData(const TLogRecord& rec) {
    Slave_->write(rec.Data, rec.Len);
}

void TStreamLogBackend::ReopenLog() {
}

TStreamWithContextLogBackend::TStreamWithContextLogBackend(std::ostream* slave)
    : Slave_(slave)
{
}

TStreamWithContextLogBackend::~TStreamWithContextLogBackend() {
}

void TStreamWithContextLogBackend::WriteData(const TLogRecord& rec) {
    Slave_->write(rec.Data, rec.Len);
    Slave_->write(DELIMITER.data(), DELIMITER.size());
    for (const auto& [key, value] : rec.MetaFlags) {
        TStringBuilder builder;
        builder << key << "=" << value;
        Slave_->write(builder.c_str(), builder.size());
        Slave_->write(DELIMITER.data(), DELIMITER.size());
    }
}

void TStreamWithContextLogBackend::ReopenLog() {
}
