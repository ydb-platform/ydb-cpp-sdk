from util.generic.ptr cimport THolder
from util.generic.string cimport std::string, std::string_view
from util.stream.output cimport IOutputStream


cdef extern from "<src/util/stream/str.h>" nogil:
    cdef cppclass TStringOutput(IOutputStream):
        TStringOutput() except+
        TStringOutput(std::string&) except+
        void Reserve(size_t) except+

ctypedef THolder[TStringOutput] TStringOutputPtr
