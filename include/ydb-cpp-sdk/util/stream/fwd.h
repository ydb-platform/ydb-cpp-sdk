#pragma once

<<<<<<< HEAD
=======
<<<<<<<< HEAD:src/util/stream/fwd.h
>>>>>>> origin/main
<<<<<<<< HEAD:include/ydb-cpp-sdk/util/stream/fwd.h
#include <ydb-cpp-sdk/util/system/types.h>
========
#include <src/util/system/types.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/stream/fwd.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/util/stream/fwd.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD
=======
========
#include <ydb-cpp-sdk/util/system/types.h>
>>>>>>>> origin/main:include/ydb-cpp-sdk/util/stream/fwd.h
>>>>>>> origin/main

class IInputStream;
class IOutputStream;

class IZeroCopyInput;
class IZeroCopyInputFastReadTo;
class IZeroCopyOutput;

using TStreamManipulator = void (*)(IOutputStream&);

class TLengthLimitedInput;

class TMemoryInput;
class TMemoryOutput;
class TMemoryWriteBuffer;

class TMultiInput;

class TNullInput;
class TNullOutput;
class TNullIO;

class TPipeBase;
class TPipeInput;
class TPipeOutput;
class TPipedBase;
class TPipedInput;
class TPipedOutput;

class TStringInput;
class TStringOutput;
class TStringStream;

class TTeeOutput;

class TTempBufOutput;

class IWalkInput;

struct TZLibError;
struct TZLibCompressorError;
struct TZLibDecompressorError;

namespace ZLib {
    enum StreamType: ui8;
}

class TZLibDecompress;
class TZLibCompress;

class TBufferInput;
class TBufferOutput;
class TBufferStream;

class TBufferedInput;
class TBufferedOutputBase;
class TBufferedOutput;
class TAdaptiveBufferedOutput;

template <class TSlave>
class TBuffered;

template <class TSlave>
class TAdaptivelyBuffered;

class TUnbufferedFileInput;
class TUnbufferedFileOutput;

class TFileInput;
using TIFStream = TFileInput;

class TFixedBufferFileOutput;
using TOFStream = TFixedBufferFileOutput;

using TFileOutput = TAdaptivelyBuffered<TUnbufferedFileOutput>;
