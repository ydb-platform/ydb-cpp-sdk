#pragma once

#include <string_view>

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/yt/yson/consumer.h
#include <ydb-cpp-sdk/util/system/defaults.h>

#include <ydb-cpp-sdk/library/yt/yson_string/public.h>
========
#include <src/util/system/defaults.h>

#include <src/library/yt/yson_string/public.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/yt/yson/consumer.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/yt/yson/consumer.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYT::NYson {

////////////////////////////////////////////////////////////////////////////////

//! A SAX-like interface for parsing a YSON stream.
struct IYsonConsumer
{
    virtual ~IYsonConsumer() = default;

    //! The current item is a string scalar (IStringNode).
    /*!
     *  \param value A scalar value.
     */
    virtual void OnStringScalar(std::string_view value) = 0;

    //! The current item is an integer scalar (IInt64Node).
    /*!
     *  \param value A scalar value.
     */
    virtual void OnInt64Scalar(i64 value) = 0;

    //! The current item is an integer scalar (IUint64Node).
    /*!
     *  \param value A scalar value.
     */
    virtual void OnUint64Scalar(ui64 value) = 0;

    //! The current item is an FP scalar (IDoubleNode).
    /*!
     *  \param value A scalar value.
     */
    virtual void OnDoubleScalar(double value) = 0;

    //! The current item is an boolean scalar (IBooleanNode).
    /*!
     *  \param value A scalar value.
     */
    virtual void OnBooleanScalar(bool value) = 0;

    //! The current item is an entity (IEntityNode).
    virtual void OnEntity() = 0;

    //! Starts a list (IListNode).
    /*!
     *  The events describing a list are raised as follows:
     *  - #OnBeginList
     *  - For each item: #OnListItem followed by the description of the item
     *  - #OnEndList
     */
    virtual void OnBeginList() = 0;

    //! Designates a list item.
    virtual void OnListItem() = 0;

    //! Ends the current list.
    virtual void OnEndList() = 0;

    //! Starts a map (IMapNode).
    /*!
     *  The events describing a map are raised as follows:
     *  - #OnBeginMap
     *  - For each item: #OnKeyedItem followed by the description of the item
     *  - #OnEndMap
     */
    virtual void OnBeginMap() = 0;

    //! Designates a keyed item (in map or in attributes).
    /*!
     *  \param key Item key in the map.
     */
    virtual void OnKeyedItem(std::string_view key) = 0;

    //! Ends the current map.
    virtual void OnEndMap() = 0;

    //! Starts attributes.
    /*!
     *  An arbitrary node may be preceded by attributes.
     *
     *  The events describing attributes are raised as follows:
     *  - #OnBeginAttributes
     *  - For each item: #OnKeyedItem followed by the description of the item
     *  - #OnEndAttributes
     */
    virtual void OnBeginAttributes() = 0;

    //! Ends the current attribute list.
    virtual void OnEndAttributes() = 0;

    //! Inserts YSON-serialized node or fragment.
    /*!
     *  \param yson Serialized data.
     *  \param type Type of data.
     */
    virtual void OnRaw(std::string_view yson, EYsonType type) = 0;

    // Extension methods.
    void OnRaw(const TYsonStringBuf& yson);
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NYson
