#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/ptr.h>
>>>>>>> origin/main


namespace NTesting {

    //@brief network port holder interface
    class IPort {
    public:
        virtual ~IPort() {}

        virtual ui16 Get() = 0;
    };

    class TPortHolder : private THolder<IPort> {
        using TBase = THolder<IPort>;
    public:
        using TBase::TBase;
        using TBase::Release;
        using TBase::Reset;

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
