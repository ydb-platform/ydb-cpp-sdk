#pragma once

#include <src/library/http/misc/httpcodes.h>
#include <src/library/http/io/stream.h>

#include <vector>

class THttpHeaders;
class IOutputStream;

class THttpResponse {
public:
    THttpResponse() noexcept
        : Code(HTTP_OK)
    {
    }

    explicit THttpResponse(HttpCodes code) noexcept
        : Code(code)
    {
    }

    template <typename ValueType>
    THttpResponse& AddHeader(const std::string& name, const ValueType& value) {
        return AddHeader(THttpInputHeader(name, ToString(value)));
    }

    THttpResponse& AddHeader(const THttpInputHeader& header) {
        Headers.AddHeader(header);

        return *this;
    }

    template <typename ValueType>
    THttpResponse& AddOrReplaceHeader(const std::string& name, const ValueType& value) {
        return AddOrReplaceHeader(THttpInputHeader(name, ToString(value)));
    }

    THttpResponse& AddOrReplaceHeader(const THttpInputHeader& header) {
        Headers.AddOrReplaceHeader(header);

        return *this;
    }

    THttpResponse& AddMultipleHeaders(const THttpHeaders& headers);

    const THttpHeaders& GetHeaders() const {
        return Headers;
    }

    THttpResponse& SetContentType(const std::string_view& contentType);

    /**
     * @note If @arg content isn't empty its size is automatically added as a
     * "Content-Length" header during output to IOutputStream.
     * @see IOutputStream& operator << (IOutputStream&, const THttpResponse&)
     */
    THttpResponse& SetContent(const std::string& content) {
        Content = content;

        return *this;
    }

    THttpResponse& SetContent(std::string&& content) {
        Content = std::move(content);

        return *this;
    }

    std::string GetContent() const {
        return Content;
    }

    /**
     * @note If @arg content isn't empty its size is automatically added as a
     * "Content-Length" header during output to IOutputStream.
     * @see IOutputStream& operator << (IOutputStream&, const THttpResponse&)
     */
    THttpResponse& SetContent(const std::string& content, const std::string_view& contentType) {
        return SetContent(content).SetContentType(contentType);
    }

    HttpCodes HttpCode() const {
        return Code;
    }

    THttpResponse& SetHttpCode(HttpCodes code) {
        Code = code;
        return *this;
    }

    void OutTo(IOutputStream& out) const;

private:
    HttpCodes Code;
    THttpHeaders Headers;
    std::string Content;
};
