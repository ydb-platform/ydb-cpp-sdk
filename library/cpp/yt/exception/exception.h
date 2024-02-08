#pragma once

#include <util/generic/string.h>

#include <exception>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////
// These are poor man's versions of NYT::TErrorException to be used in
// a limited subset of core libraries that are needed to implement NYT::TError.

class TSimpleException
    : public std::exception
{
public:
    explicit TSimpleException(std::string message);

    const std::string& GetMessage() const;
    const char* what() const noexcept override;

protected:
    const std::string Message_;
};

class TCompositeException
    : public TSimpleException
{
public:
    explicit TCompositeException(std::string message);
    TCompositeException(
        const std::exception& exception,
        std::string message);

    const std::exception_ptr& GetInnerException() const;
    const char* what() const noexcept override;

private:
    const std::exception_ptr InnerException_;
    const std::string What_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
