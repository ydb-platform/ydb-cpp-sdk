#pragma once

#include "duration_meter.h"
#include "records.h"
#include "shard.h"
#include "slo_text_utils.h"

#include <util/datetime/base.h>
#include <util/generic/guid.h>
#include <util/random/random.h>

#include <functional>
#include <unordered_map>
#include <vector>

class TValueGenerator {
public:
    TValueGenerator(const TSloGeneratorOptions& opts, std::uint32_t startId = 0);
    TRecordData Get();
    TDuration GetComputeTime() const;

private:
    TSloGeneratorOptions Opts;
    std::uint32_t CurrentObjectId = 0;
    TDuration ComputeTime;
};

class TKeyValueGenerator {
public:
    TKeyValueGenerator(const TSloGeneratorOptions& opts, std::uint32_t startId = 0);
    TKeyValueRecordData Get();
    TDuration GetComputeTime() const;

private:
    TSloGeneratorOptions Opts;
    std::uint32_t CurrentObjectId = 0;
    TDuration ComputeTime;
};

template<typename TGeneratorType, typename TRecordType, typename TItem>
class TPackGenerator {
public:
    using TBuildItem = std::function<TItem(const TRecordType&)>;

    TPackGenerator(
        const TSloGeneratorOptions& opts,
        std::uint32_t packSize,
        TBuildItem buildItem,
        std::uint64_t remain,
        std::uint32_t startId = 0
    )
        : BuildItem_(std::move(buildItem))
        , Generator_(opts, startId)
        , PackSize_(packSize)
        , Remain_(remain)
    {
    }

    bool GetNextPack(std::vector<TItem>& pack) {
        pack.clear();
        while (Remain_) {
            TRecordType record = Generator_.Get();
            --Remain_;
            const std::uint32_t specialId = SloGetSpecialId(SloGetHash(record.ObjectId));
            auto& existingPack = Packs_[specialId];
            existingPack.emplace_back(BuildItem_(record));
            if (existingPack.size() >= PackSize_) {
                existingPack.swap(pack);
                return true;
            }
        }
        for (auto& it : Packs_) {
            if (it.second.size()) {
                it.second.swap(pack);
                return true;
            }
        }
        return false;
    }

    std::uint32_t GetPackSize() const {
        return PackSize_;
    }

    TDuration GetComputeTime() const {
        return Generator_.GetComputeTime();
    }

    std::uint64_t GetRemain() const {
        return Remain_;
    }

private:
    TBuildItem BuildItem_;
    TGeneratorType Generator_;
    std::uint32_t PackSize_;
    std::unordered_map<std::uint32_t, std::vector<TItem>> Packs_;
    std::uint64_t Remain_;
};
