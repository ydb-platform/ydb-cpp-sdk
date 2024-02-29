from util.generic.string cimport TString

from libcpp cimport bool as bool_t

cdef extern from "<util/string/cast.h>" nogil:
    T FromString[T](const std::string&) except +
    bool_t TryFromString[T](const std::string&, T&) except +
    std::string ToString[T](const T&) except +

    cdef double StrToD(const char* b, char** se) except +
