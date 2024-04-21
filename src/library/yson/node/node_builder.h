#pragma once

#include <ydb-cpp-sdk/library/yson/node/node.h>

#include <ydb-cpp-sdk/library/json/json_reader.h>

#include <ydb-cpp-sdk/library/yson/consumer.h>

#include <src/util/generic/stack.h>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

class TNodeBuilder
    : public ::NYson::TYsonConsumerBase
{
public:
    TNodeBuilder(TNode* node);

    void OnStringScalar(std::string_view) override;
    void OnInt64Scalar(i64) override;
    void OnUint64Scalar(ui64) override;
    void OnDoubleScalar(double) override;
    void OnBooleanScalar(bool) override;
    void OnEntity() override;
    void OnBeginList() override;
    void OnBeginList(ui64 reserveSize);
    void OnListItem() override;
    void OnEndList() override;
    void OnBeginMap() override;
    void OnBeginMap(ui64 reserveSize);
    void OnKeyedItem(std::string_view) override;
    void OnEndMap() override;
    void OnBeginAttributes() override;
    void OnEndAttributes() override;
    void OnNode(TNode node);

private:
    TStack<TNode*> Stack_;

private:
    inline void AddNode(TNode node, bool pop);
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
