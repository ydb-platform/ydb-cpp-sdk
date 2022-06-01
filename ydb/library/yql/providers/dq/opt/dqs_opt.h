#pragma once

#include <ydb/library/yql/core/yql_graph_transformer.h>

namespace NKikimr::NMiniKQL {
    class IFunctionRegistry;
}

namespace NYql {
    struct TTypeAnnotationContext;
}

namespace NYql::NDqs {
    class TDatabaseManager;

    THolder<IGraphTransformer> CreateDqsWrapListsOptTransformer();
    THolder<IGraphTransformer> CreateDqsFinalizingOptTransformer();
    THolder<IGraphTransformer> CreateDqsBuildTransformer();
    THolder<IGraphTransformer> CreateDqsRewritePhyCallablesTransformer();
    THolder<IGraphTransformer> CreateDqsReplacePrecomputesTransformer(TTypeAnnotationContext* typesCtx, const NKikimr::NMiniKQL::IFunctionRegistry* funcRegistry);
    THolder<IGraphTransformer> CreateDqsPeepholeTransformer(THolder<IGraphTransformer>&& typeAnnTransformer, TTypeAnnotationContext& typesCtx);

} // namespace NYql::NDqs
