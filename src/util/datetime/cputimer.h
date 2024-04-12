#pragma once

#include <chrono>

class TTimer {
    using TClock = std::chrono::high_resolution_clock;

    TClock Clock_; 
    std::chrono::time_point<TClock, TClock::duration> Start_;
    std::stringstream Message_;

public:
    TTimer(const std::string_view message = " took: ");
    ~TTimer();
};

class TFuncTimer {
    using TClock = std::chrono::high_resolution_clock;
public:
    TFuncTimer(const char* func);
    ~TFuncTimer();

private:
    TClock Clock_; 
    const std::chrono::time_point<TClock, TClock::duration> Start_;
    const char* Func_;
};
