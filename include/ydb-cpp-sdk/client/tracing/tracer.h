#pragma once

#include <memory>
#include <string>

namespace NYdb::inline V3 {
namespace NTracing {

class ISpan {
public:
    virtual ~ISpan() = default;

    virtual void AddAttribute(const std::string& key, const std::string& value) = 0;
    virtual void End() = 0;
};

class ITracer {
public:
    virtual ~ITracer() = default;

    virtual std::unique_ptr<ISpan> StartSpan(const std::string& name) = 0;
};

} // namespace NTracing
} // namespace NYdb
