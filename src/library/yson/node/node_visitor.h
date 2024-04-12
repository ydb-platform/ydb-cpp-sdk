#pragma once

#include <ydb-cpp-sdk/library/yson/node/node.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/yson/consumer.h>
=======
#include <src/library/yson/consumer.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

class TNodeVisitor
{
public:
    TNodeVisitor(NYson::IYsonConsumer* consumer, bool sortMapKeys = false);

    void Visit(const TNode& node);
    void VisitMap(const TNode::TMapType& nodeMap);
    void VisitList(const TNode::TListType& nodeMap);

private:
    NYson::IYsonConsumer* Consumer_;
    bool SortMapKeys_;

private:
    void VisitAny(const TNode& node);

    void VisitString(const TNode& node);
    void VisitInt64(const TNode& node);
    void VisitUint64(const TNode& node);
    void VisitDouble(const TNode& node);
    void VisitBool(const TNode& node);
    void VisitEntity();
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
