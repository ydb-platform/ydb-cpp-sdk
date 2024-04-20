#include "ptr.h"

#include <util/system/defaults.h>
#include <util/memory/alloc.h>

#include <new>
#include <cstdlib>

/*
void TFree::DoDestroy(void* t) noexcept {
    free(t);
}
*/

/*void TFree::DoDestroy(void* t) noexcept {
    delete static_cast<char*>(t); // Удаляем объект, приведенный к типу char*
}*/

void TDelete::Destroy(void* t) noexcept {
    ::operator delete(t);
}

TThrRefBase::~TThrRefBase() = default;
