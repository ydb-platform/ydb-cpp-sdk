#pragma once

<<<<<<< HEAD
<<<<<<<< HEAD:include/ydb-cpp-sdk/library/http/io/headers.h
#include <ydb-cpp-sdk/util/string/builder.h>
========
#include <src/util/string/builder.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/http/io/headers.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/http/io/headers.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/string/builder.h>
>>>>>>> origin/main

#include <string>
#include <string_view>

<<<<<<< HEAD
<<<<<<<< HEAD:include/ydb-cpp-sdk/library/http/io/headers.h
#include <ydb-cpp-sdk/util/string/cast.h>
========
#include <src/util/string/cast.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/http/io/headers.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/http/io/headers.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/string/cast.h>
>>>>>>> origin/main

class IInputStream;
class IOutputStream;

/// @addtogroup Streams_HTTP
/// @{
/// Объект, содержащий информацию о HTTP-заголовке.
class THttpInputHeader {
public:
    THttpInputHeader() = delete;
    THttpInputHeader(const THttpInputHeader&) = default;
    THttpInputHeader(THttpInputHeader&&) = default;
    THttpInputHeader& operator=(const THttpInputHeader&) = default;
    THttpInputHeader& operator=(THttpInputHeader&&) = default;

    /// @param[in] header - строка вида 'параметр: значение'.
    THttpInputHeader(std::string_view header);
    /// @param[in] name - имя параметра.
    /// @param[in] value - значение параметра.
    THttpInputHeader(std::string name, std::string value);

    /// Возвращает имя параметра.
    inline const std::string& Name() const noexcept {
        return Name_;
    }

    /// Возвращает значение параметра.
    inline const std::string& Value() const noexcept {
        return Value_;
    }

    /// Записывает заголовок вида "имя параметра: значение\r\n" в поток.
    void OutTo(IOutputStream* stream) const;

    /// Возвращает строку "имя параметра: значение".
    inline std::string ToString() const {
        return TStringBuilder() << Name_ << ": " << Value_;
    }

private:
    std::string Name_;
    std::string Value_;
};

/// Контейнер для хранения HTTP-заголовков
class THttpHeaders {
    using THeaders = std::deque<THttpInputHeader>;

public:
    using TConstIterator = THeaders::const_iterator;

    THttpHeaders() = default;
    THttpHeaders(const THttpHeaders&) = default;
    THttpHeaders& operator=(const THttpHeaders&) = default;
    THttpHeaders(THttpHeaders&&) = default;
    THttpHeaders& operator=(THttpHeaders&&) = default;

    /// Добавляет каждую строку из потока в контейнер, считая ее правильным заголовком.
    THttpHeaders(IInputStream* stream);

    /// Стандартный итератор.
    inline TConstIterator Begin() const noexcept {
        return Headers_.begin();
    }
    inline TConstIterator begin() const noexcept {
        return Headers_.begin();
    }

    /// Стандартный итератор.
    inline TConstIterator End() const noexcept {
        return Headers_.end();
    }
    inline TConstIterator end() const noexcept {
        return Headers_.end();
    }

    /// Возвращает количество заголовков в контейнере.
    inline size_t Count() const noexcept {
        return Headers_.size();
    }

    /// Проверяет, содержит ли контейнер хотя бы один заголовок.
    inline bool Empty() const noexcept {
        return Headers_.empty();
    }

    /// Добавляет заголовок в контейнер.
    void AddHeader(THttpInputHeader header);

    template <typename ValueType>
    void AddHeader(std::string name, const ValueType& value) {
        AddHeader(THttpInputHeader(std::move(name), ToString(value)));
    }

    /// Добавляет заголовок в контейнер, если тот не содержит заголовка
    /// c таким же параметром. В противном случае, заменяет существующий
    /// заголовок на новый.
    void AddOrReplaceHeader(const THttpInputHeader& header);

    template <typename ValueType>
    void AddOrReplaceHeader(std::string name, const ValueType& value) {
        AddOrReplaceHeader(THttpInputHeader(std::move(name), ToString(value)));
    }

    // Проверяет, есть ли такой заголовок
    bool HasHeader(std::string_view header) const;

    /// Удаляет заголовок, если он есть.
    void RemoveHeader(std::string_view header);

    /// Ищет заголовок по указанному имени
    /// Возвращает nullptr, если не нашел
    const THttpInputHeader* FindHeader(std::string_view header) const;

    /// Записывает все заголовки контейнера в поток.
    /// @details Каждый заголовк записывается в виде "имя параметра: значение\r\n".
    void OutTo(IOutputStream* stream) const;

    /// Обменивает наборы заголовков двух контейнеров.
    void Swap(THttpHeaders& headers) noexcept {
        Headers_.swap(headers.Headers_);
    }

private:
    THeaders Headers_;
};

/// @}
