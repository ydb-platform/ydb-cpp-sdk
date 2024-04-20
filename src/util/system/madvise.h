#pragma once

#include <ydb-cpp-sdk/util/system/defaults.h>

#include <span>

/// see linux madvise(MADV_SEQUENTIAL)
void MadviseSequentialAccess(const void* begin, size_t size);
void MadviseSequentialAccess(std::span<const char> data);
void MadviseSequentialAccess(std::span<const ui8> data);

/// see linux madvise(MADV_RANDOM)
void MadviseRandomAccess(const void* begin, size_t size);
void MadviseRandomAccess(std::span<const char> data);
void MadviseRandomAccess(std::span<const ui8> data);

/// see linux madvise(MADV_DONTNEED)
void MadviseEvict(const void* begin, size_t size);
void MadviseEvict(std::span<const char> data);
void MadviseEvict(std::span<const ui8> data);

/// see linux madvise(MADV_DONTDUMP)
void MadviseExcludeFromCoreDump(const void* begin, size_t size);
void MadviseExcludeFromCoreDump(std::span<const char> data);
void MadviseExcludeFromCoreDump(std::span<const ui8> data);

/// see linux madvise(MADV_DODUMP)
void MadviseIncludeIntoCoreDump(const void* begin, size_t size);
void MadviseIncludeIntoCoreDump(std::span<const char> data);
void MadviseIncludeIntoCoreDump(std::span<const ui8> data);
