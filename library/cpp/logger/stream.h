#pragma once

#include "backend.h"

#include <string_view>

class IOutputStream;

class TStreamLogBackend : public TLogBackend {
public:
    explicit TStreamLogBackend(std::ostream* slave);
    ~TStreamLogBackend() override;

    void WriteData(const TLogRecord& rec) override;
    void ReopenLog() override;

private:
    std::ostream* Slave_;
};

class TStreamWithContextLogBackend : public TLogBackend {
private:
    static constexpr std::string_view DELIMITER = "; ";

public:
    explicit TStreamWithContextLogBackend(std::ostream* slave);
    ~TStreamWithContextLogBackend() override;

    void WriteData(const TLogRecord& rec) override;
    void ReopenLog() override;

private:
    std::ostream* Slave_;
};
