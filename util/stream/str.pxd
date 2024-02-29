from util.generic.ptr cimport THolder
from util.generic.string cimport TString, std::string_view
from util.stream.output cimport IOutputStream


cdef extern from "<util/stream/str.h>" nogil:
    cdef cppclass TStringOutput(IOutputStream):
        TStringOutput() except+
        TStringOutput(std::string&) except+
        void Reserve(size_t) except+

ctypedef THolder[TStringOutput] TStringOutputPtr
