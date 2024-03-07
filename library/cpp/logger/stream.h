#pragma once

#include "backend.h"

#include <string>


class IOutputStream;

class TStreamLogBackend : public TLogBackend {
public:
    explicit TStreamLogBackend(IOutputStream* slave);
    ~TStreamLogBackend() override;

    void WriteData(const TLogRecord& rec) override;
    void ReopenLog() override;

private:
    IOutputStream* Slave_;
};

class TStreamWithContextLogBackend : public TLogBackend {
private:
    static constexpr std::string_view DELIMITER = "; ";

public:
    explicit TStreamWithContextLogBackend(IOutputStream* slave);
    ~TStreamWithContextLogBackend() override;

    void WriteData(const TLogRecord& rec) override;
    void ReopenLog() override;

private:
    IOutputStream* Slave_;
};
