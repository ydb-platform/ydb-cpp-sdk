#include <ydb-cpp-sdk/library/yt/exception/exception.h>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

TSimpleException::TSimpleException(std::string message)
    : Message_(std::move(message))
{ }

const std::string& TSimpleException::GetMessage() const
{
    return Message_;
}

const char* TSimpleException::what() const noexcept
{
    return Message_.c_str();
}

////////////////////////////////////////////////////////////////////////////////

TCompositeException::TCompositeException(std::string message)
    : TSimpleException(std::move(message))
    , What_(Message_)
{ }

TCompositeException::TCompositeException(
    const std::exception& exception,
    std::string message)
    : TSimpleException(message)
    , InnerException_(std::current_exception())
    , What_(message + "\n" + exception.what())
{ }

const std::exception_ptr& TCompositeException::GetInnerException() const
{
    return InnerException_;
}

const char* TCompositeException::what() const noexcept
{
    return What_.c_str();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
