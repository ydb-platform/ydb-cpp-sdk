#pragma once

#include <ydb-cpp-sdk/util/generic/ptr.h>

#include <memory>

namespace NTesting {

    //@brief network port holder interface
    class IPort {
    public:
        virtual ~IPort() {}

        virtual ui16 Get() = 0;
    };

    class TPortHolder : private std::unique_ptr<IPort> {
        using TBase = std::unique_ptr<IPort>;
    public:
        using TBase::TBase;
        using TBase::release;
        using TBase::reset;

        TPortHolder(std::unique_ptr<IPort>&& port);

        operator ui16() const& {
            return (*this)->Get();
        }

        operator ui16() const&& = delete;
    };

    IOutputStream& operator<<(IOutputStream& out, const TPortHolder& port);

    //@brief Get first free port.
    [[nodiscard]] TPortHolder GetFreePort();

    namespace NLegacy {
        // Do not use these methods made for Unittest TPortManager backward compatibility.
        // Returns continuous sequence of the specified number of ports.
        [[nodiscard]] std::vector<TPortHolder> GetFreePortsRange(size_t count);
        //@brief Returns port from parameter if NO_RANDOM_PORTS env var is set, otherwise first free port
        [[nodiscard]] TPortHolder GetPort(ui16 port);
    }

    //@brief Reinitialize singleton from environment vars for tests
    void InitPortManagerFromEnv();

    //@brief helper class for inheritance
    struct TFreePortOwner {
        TFreePortOwner() : Port_(GetFreePort()) {}

        ui16 GetPort() const {
            return Port_;
        }

    private:
        TPortHolder Port_;
    };
}
