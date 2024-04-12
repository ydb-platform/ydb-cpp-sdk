#pragma once

#include "cartesian_product.h"
#include "concatenate.h"
#include "enumerate.h"
#include "filtering.h"
#include "mapped.h"
#include "zip.h"

#include <src/util/generic/adaptor.h>
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/xrange.h>
=======
#include <src/util/generic/xrange.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <tuple>
#include <algorithm>


namespace NFuncTools {
    using ::Enumerate;
    using ::Reversed;
    using ::Zip;
    using ::Concatenate;
    using ::CartesianProduct;

    template <typename TValue>
    auto Range(TValue from, TValue to, TValue step) {
        return xrange(from, to, step);
    }

    template <typename TValue>
    auto Range(TValue from, TValue to) {
        return xrange(from, to);
    }

    template <typename TValue>
    auto Range(TValue to) {
        return xrange(to);
    }

    //! Usage: for (i32 x : Map([](i32 x) { return x * x; }, a)) {...}
    template <typename TMapper, typename TContainerOrRef>
    auto Map(TMapper&& mapper, TContainerOrRef&& container) {
        return ::MakeMappedRange(std::forward<TContainerOrRef>(container), std::forward<TMapper>(mapper));
    }

    //! Usage: for (auto i : Map<int>(floats)) {...}
    template <typename TMapResult, typename TContainerOrRef>
    auto Map(TContainerOrRef&& container) {
        return Map([](const auto& x) { return TMapResult(x); }, std::forward<TContainerOrRef>(container));
    }

    //! Usage: for (i32 x : Filter(predicate, container)) {...}
    template <typename TPredicate, typename TContainerOrRef>
    auto Filter(TPredicate&& predicate, TContainerOrRef&& container) {
        return ::MakeFilteringRange(std::forward<TContainerOrRef>(container), std::forward<TPredicate>(predicate));
    }

}
