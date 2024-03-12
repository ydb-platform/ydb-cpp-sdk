#pragma once

#include <functional>

class IThreadFactory;

struct IObjectInQueue;
class TThreadFactoryHolder;

using TThreadFunction = std::function<void()>;

class IThreadPool;
class TFakeThreadPool;
class TThreadPool;
class TAdaptiveThreadPool;
class TSimpleThreadPool;

template <class TQueueType, class TSlave>
class TThreadPoolBinder;
