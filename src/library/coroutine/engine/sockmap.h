#pragma once

#include <vector>
#include <unordered_map>

template <class T>
class TSocketMap {
public:
    T& Get(size_t idx) {
        if (idx < 128000) {
            if (V_.size() <= idx) {
                V_.resize(idx + 1);
            }

            return V_[idx];
        }

        return H_[idx];
    }

private:
    std::vector<T> V_;
    std::unordered_map<size_t, T> H_;
};
