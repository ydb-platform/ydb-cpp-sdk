from libcpp.string cimport string as _std_string

cdef extern from "<util/generic/strbuf.h>" nogil:

    cdef cppclass std::string_view:
        std::string_view() except +
        std::string_view(const char*) except +
        std::string_view(const char*, size_t) except +
        const char* data()
        char* Data()
        size_t size()
        size_t Size()


cdef extern from "<util/generic/string.h>" nogil:

    size_t npos "TString::npos"

    # Inheritance is bogus, but it's safe to assume std::string is-a std::string_view via implicit cast
    cdef cppclass std::string(std::string_view):
        std::string() except +
        std::string(std::string&) except +
        std::string(_std_string&) except +
        std::string(std::string&, size_t, size_t) except +
        std::string(char*) except +
        std::string(char*, size_t) except +
        std::string(char*, size_t, size_t) except +
        # as a std::string formed by a repetition of character c, n times.
        std::string(size_t, char) except +
        std::string(char*, char*) except +
        std::string(std::string_view&) except +
        std::string(std::string_view&, std::string_view&) except +
        std::string(std::string_view&, std::string_view&, std::string_view&) except +

        const char* c_str()
        size_t max_size()
        size_t length()
        void resize(size_t) except +
        void resize(size_t, char c) except +
        size_t capacity()
        void reserve(size_t) except +
        void clear() except +
        bint empty()

        char& at(size_t)
        char& operator[](size_t)
        int compare(std::string_view&)

        std::string& append(std::string_view&) except +
        std::string& append(std::string_view&, size_t, size_t) except +
        std::string& append(char *) except +
        std::string& append(char *, size_t) except +
        std::string& append(size_t, char) except +

        void push_back(char c) except +

        std::string& assign(std::string_view&) except +
        std::string& assign(std::string_view&, size_t, size_t) except +
        std::string& assign(char *) except +
        std::string& assign(char *, size_t) except +

        std::string& insert(size_t, std::string&) except +
        std::string& insert(size_t, std::string&, size_t, size_t) except +
        std::string& insert(size_t, char* s) except +
        std::string& insert(size_t, char* s, size_t) except +
        std::string& insert(size_t, size_t, char c) except +

        size_t copy(char *, size_t) except +
        size_t copy(char *, size_t, size_t) except +

        size_t find(std::string_view&)
        size_t find(std::string_view&, size_t pos)
        size_t find(char)
        size_t find(char, size_t pos)

        size_t rfind(std::string_view&)
        size_t rfind(std::string_view&, size_t pos)
        size_t rfind(char)
        size_t rfind(char, size_t pos)

        size_t find_first_of(char c)
        size_t find_first_of(char c, size_t pos)
        size_t find_first_of(std::string_view& set)
        size_t find_first_of(std::string_view& set, size_t pos)

        size_t find_first_not_of(char c)
        size_t find_first_not_of(char c, size_t pos)
        size_t find_first_not_of(std::string_view& set)
        size_t find_first_not_of(std::string_view& set, size_t pos)

        size_t find_last_of(char c)
        size_t find_last_of(char c, size_t pos)
        size_t find_last_of(std::string_view& set)
        size_t find_last_of(std::string_view& set, size_t pos)

        std::string substr(size_t pos) except +
        std::string substr(size_t pos, size_t n) except +

        std::string operator+(std::string_view& rhs) except +
        std::string operator+(char* rhs) except +

        bint operator==(std::string_view&)
        bint operator==(char*)

        bint operator!=(std::string_view&)
        bint operator!=(char*)

        bint operator<(std::string_view&)
        bint operator<(char*)

        bint operator>(std::string_view&)
        bint operator>(char*)

        bint operator<=(std::string_view&)
        bint operator<=(char*)

        bint operator>=(std::string_view&)
        bint operator>=(char*)
