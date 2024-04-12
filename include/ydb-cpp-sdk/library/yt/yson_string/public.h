#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/yt/yson_string/public.h
#include <ydb-cpp-sdk/library/yt/misc/enum.h>
========
#include <src/library/yt/misc/enum.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/yt/yson_string/public.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/yt/yson_string/public.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYT::NYson {

////////////////////////////////////////////////////////////////////////////////

//! The data format.
DEFINE_ENUM(EYsonFormat,
    // Binary.
    // Most compact but not human-readable.
    (Binary)

    // Text.
    // Not so compact but human-readable.
    // Does not use indentation.
    // Uses escaping for non-text characters.
    (Text)

    // Text with indentation.
    // Extremely verbose but human-readable.
    // Uses escaping for non-text characters.
    (Pretty)
);

// NB: -1 is used for serializing null TYsonString.
DEFINE_ENUM_WITH_UNDERLYING_TYPE(EYsonType, i8,
    ((Node)          (0))
    ((ListFragment)  (1))
    ((MapFragment)   (2))
);

class TYsonString;
class TYsonStringBuf;

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NYson
