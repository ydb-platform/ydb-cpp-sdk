#pragma once

#include "node.h"
<<<<<<<< HEAD:include/ydb-cpp-sdk/library/yson/node/node_io.h
#include <ydb-cpp-sdk/library/yson/public.h>
========
#include <src/library/yson/public.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/yson/node/node_io.h

namespace NJson {
    class TJsonValue;
} // namespace NJson

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

// Parse TNode from string in YSON format
TNode NodeFromYsonString(const std::string_view input, ::NYson::EYsonType type = ::NYson::EYsonType::Node);

// Serialize TNode to string in one of YSON formats with random order of maps' keys (don't use in tests)
std::string NodeToYsonString(const TNode& node, ::NYson::EYsonFormat format = ::NYson::EYsonFormat::Text);

// Same as the latter, but maps' keys are sorted lexicographically (to be used in tests)
std::string NodeToCanonicalYsonString(const TNode& node, ::NYson::EYsonFormat format = ::NYson::EYsonFormat::Text);

// Parse TNode from stream in YSON format
TNode NodeFromYsonStream(IInputStream* input, ::NYson::EYsonType type = ::NYson::EYsonType::Node);

// Serialize TNode to stream in one of YSON formats with random order of maps' keys (don't use in tests)
void NodeToYsonStream(const TNode& node, IOutputStream* output, ::NYson::EYsonFormat format = ::NYson::EYsonFormat::Text);

// Same as the latter, but maps' keys are sorted lexicographically (to be used in tests)
void NodeToCanonicalYsonStream(const TNode& node, IOutputStream* output, ::NYson::EYsonFormat format = ::NYson::EYsonFormat::Text);

// Parse TNode from string in JSON format
TNode NodeFromJsonString(const std::string_view input);
bool TryNodeFromJsonString(const std::string_view input, TNode& dst);

// Parse TNode from string in JSON format using an iterative JSON parser.
// Iterative JSON parsers still use the stack, but allocate it on the heap (instead of using the system call stack).
// Needed to mitigate stack overflow with short stacks on deeply nested JSON strings
//  (e.g. 256kb of stack when parsing "[[[[[[...]]]]]]" crashes the whole binary).
TNode NodeFromJsonStringIterative(const std::string_view input, ui64 maxDepth = 1024);

// Convert TJsonValue to TNode
TNode NodeFromJsonValue(const ::NJson::TJsonValue& input);

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
