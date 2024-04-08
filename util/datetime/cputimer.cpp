#include "cputimer.h"

#include <util/system/defaults.h>
#include <util/system/hp_timer.h>
#include <util/stream/output.h>
#include <util/generic/singleton.h>

#include <iostream>

TTimer::TTimer(const std::string_view message) {
    Message_ << message;
    Start_ = Clock_.now();
}

TTimer::~TTimer() {
    const TClock::duration duration = Clock_.now() - Start_;
    Message_ << duration << "\n";
    std::cerr << Message_.str();
}

TFuncTimer::TFuncTimer(const char* func)
    : Start_(Clock_.now())
    , Func_(func)
{
    std::cerr << "enter " << Func_ << std::endl;
}

TFuncTimer::~TFuncTimer() {
    std::cerr << "leave " << Func_ << " -> " << Clock_.now() - Start_ << std::endl;
}
