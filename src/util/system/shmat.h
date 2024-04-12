#pragma once

#include <ydb-cpp-sdk/util/system/fhandle.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
#include <src/util/generic/guid.h>

class TSharedMemory: public TThrRefBase {
    TGUID Id;
    FHANDLE Handle;
    void* Data;
    int Size;

public:
    TSharedMemory();
    ~TSharedMemory() override;

    bool Create(int Size);
    bool Open(const TGUID& id, int size);

    const TGUID& GetId() {
        return Id;
    }

    void* GetPtr() {
        return Data;
    }

    int GetSize() const {
        return Size;
    }
};
