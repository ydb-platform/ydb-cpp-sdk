#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/input.h>
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/generic/algorithm.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/stream/input.h>
#include <src/util/stream/output.h>
#include <src/util/generic/algorithm.h>
#include <src/util/generic/ptr.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

#include <optional>

class THttpHeaders;

/// @addtogroup Streams_Chunked
/// @{
/// Ввод данных порциями.
/// @details Последовательное чтение блоков данных. Предполагается, что
/// данные записаны в виде <длина блока><блок данных>.
class TChunkedInput: public IInputStream {
public:
    /// Если передан указатель на trailers, то туда будут записаны HTTP trailer'ы (возможно пустые),
    /// которые идут после чанков.
    TChunkedInput(IInputStream* slave, std::optional<THttpHeaders>* trailers = nullptr);
    ~TChunkedInput() override;

private:
    size_t DoRead(void* buf, size_t len) override;
    size_t DoSkip(size_t len) override;

private:
    class TImpl;
    THolder<TImpl> Impl_;
};

/// Вывод данных порциями.
/// @details Вывод данных блоками в виде <длина блока><блок данных>. Если объем
/// данных превышает 64K, они записываются в виде n блоков по 64K + то, что осталось.
class TChunkedOutput: public IOutputStream {
public:
    TChunkedOutput(IOutputStream* slave);
    ~TChunkedOutput() override;

private:
    void DoWrite(const void* buf, size_t len) override;
    void DoFlush() override;
    void DoFinish() override;

private:
    class TImpl;
    THolder<TImpl> Impl_;
};
/// @}
