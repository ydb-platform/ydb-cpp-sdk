#include "hash_table.h"

#include <src/util/string/escape.h>
#include <src/util/string/cast.h>

const void* const _yhashtable_empty_data[] = {(void*)3, nullptr, (void*)1};

std::string NPrivate::MapKeyToString(std::string_view key) {
    constexpr size_t HASH_KEY_MAX_LENGTH = 500;
    try {
        return EscapeC(key.substr(0, HASH_KEY_MAX_LENGTH));
    } catch (...) {
        return "std::string_view";
    }
}

std::string NPrivate::MapKeyToString(unsigned short key) {
    return ToString(key);
}

std::string NPrivate::MapKeyToString(short key) {
    return ToString(key);
}

std::string NPrivate::MapKeyToString(unsigned int key) {
    return ToString(key);
}

std::string NPrivate::MapKeyToString(int key) {
    return ToString(key);
}

std::string NPrivate::MapKeyToString(unsigned long key) {
    return ToString(key);
}

std::string NPrivate::MapKeyToString(long key) {
    return ToString(key);
}

std::string NPrivate::MapKeyToString(unsigned long long key) {
    return ToString(key);
}

std::string NPrivate::MapKeyToString(long long key) {
    return ToString(key);
}

void NPrivate::ThrowKeyNotFoundInHashTableException(const std::string_view keyRepresentation) {
    ythrow yexception() << "Key not found in hashtable: " << keyRepresentation;
}
