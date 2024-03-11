#include "iterator.h"

#include <cstring>

static int SortFTSENTByName(const FTSENT** a, const FTSENT** b) {
    return strcmp((*a)->fts_name, (*b)->fts_name);
}

void TDirIterator::TFtsDestroy::operator() (FTS* f) noexcept {
     yfts_close(f);
}

TDirIterator::TOptions& TDirIterator::TOptions::SetSortByName() noexcept {
    return SetSortFunctor(SortFTSENTByName);
}
