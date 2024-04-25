#pragma once

#include "string.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/library/yt/misc/enum.h>
=======
#include <src/library/yt/misc/enum.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/library/yt/misc/enum.h>
>>>>>>> origin/main

#include <optional>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

std::optional<std::string> TryDecodeEnumValue(std::string_view value);
std::string DecodeEnumValue(std::string_view value);
std::string EncodeEnumValue(std::string_view value);

template <class T>
std::optional<T> TryParseEnum(std::string_view value);

template <class T>
T ParseEnum(std::string_view value);

template <class T>
void FormatEnum(TStringBuilderBase* builder, T value, bool lowerCase);

template <class T>
std::string FormatEnum(T value);

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT

#define ENUM_INL_H_
#include "enum-inl.h"
#undef ENUM_INL_H_
