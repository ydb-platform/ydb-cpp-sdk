#pragma once

#include "encoder_state_enum.h"

#include <tools/enum_parser/enum_serialization_runtime/serialized_enum.h>
<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/yexception.h>
>>>>>>> origin/main


namespace NMonitoring {

    template <typename EEncoderState>
    class TEncoderStateImpl {
    public:
        using EState = EEncoderState;

        explicit TEncoderStateImpl(EEncoderState state = EEncoderState::ROOT)
            : State_(state)
        {
        }

        TEncoderStateImpl& operator=(EEncoderState rhs) noexcept {
            State_ = rhs;
            return *this;
        }

        inline bool operator==(EEncoderState rhs) const noexcept {
            return State_ == rhs;
        }

        inline bool operator!=(EEncoderState rhs) const noexcept {
            return !operator==(rhs);
        }

        [[noreturn]] inline void ThrowInvalid(std::string_view message) const {
            ythrow yexception() << "invalid encoder state: "
                                << ToStr() << ", " << message;
        }

        inline void Expect(EEncoderState expected) const {
            if (Y_UNLIKELY(State_ != expected)) {
                ythrow yexception()
                    << "invalid encoder state: " << ToStr()
                    << ", expected: " << TEncoderStateImpl(expected).ToStr();
            }
        }

        inline void Switch(EEncoderState from, EEncoderState to) {
            Expect(from);
            State_ = to;
        }

        std::string_view ToStr() const noexcept {
            return NEnumSerializationRuntime::GetEnumNamesImpl<EEncoderState>().at(State_);
        }

    private:
        EEncoderState State_;
    };

    using TEncoderState = TEncoderStateImpl<EEncoderState>;

} // namespace NMonitoring
