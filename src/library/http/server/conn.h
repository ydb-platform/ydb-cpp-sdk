#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/http/server/conn.h
#include <ydb-cpp-sdk/library/http/io/stream.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
========
#include <src/library/http/io/stream.h>
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/http/server/conn.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/http/server/conn.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

class TSocket;

/// Потоки ввода/вывода для получения запросов и отправки ответов HTTP-сервера.
class THttpServerConn {
public:
    explicit THttpServerConn(const TSocket& s);
    THttpServerConn(const TSocket& s, size_t outputBufferSize);
    ~THttpServerConn();

    THttpInput* Input() noexcept;
    THttpOutput* Output() noexcept;

    inline const THttpInput* Input() const noexcept {
        return const_cast<THttpServerConn*>(this)->Input();
    }

    inline const THttpOutput* Output() const noexcept {
        return const_cast<THttpServerConn*>(this)->Output();
    }

    /// Проверяет, можно ли установить режим, при котором соединение с сервером
    /// не завершается после окончания транзакции.
    inline bool CanBeKeepAlive() const noexcept {
        return Output()->CanBeKeepAlive();
    }

    void Reset();

private:
    class TImpl;
    THolder<TImpl> Impl_;
};
