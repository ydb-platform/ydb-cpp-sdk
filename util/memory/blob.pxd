from libcpp cimport bool as bool_t

from util.system.types cimport ui8


cdef extern from "util/memory/blob.h" nogil:
    cdef cppclass TBlob:
        TBlob()
        TBlob(const TBlob&)
        void Swap(TBlob& r)
        const void* Data() const
        size_t Size() const
        bool_t Empty() const
        bool_t IsNull() const
        const char* AsCharPtr() const
        const unsigned char* AsUnsignedCharPtr() const
        void Drop()
        TBlob SubBlob(size_t len) except +
        TBlob SubBlob(size_t begin, size_t end) except +
        TBlob DeepCopy() except +

        @staticmethod
        TBlob NoCopy(const void* data, size_t length) except +

        @staticmethod
        TBlob Copy(const void* data, size_t length) except +

        @staticmethod
        TBlob FromFile(const std::string& path) except +

        @staticmethod
        TBlob PrechargedFromFile(const std::string& path) except +

        @staticmethod
        TBlob FromString(const std::string& s) except +

        ui8& operator[](size_t) const
        TBlob& operator=(TBlob&)


