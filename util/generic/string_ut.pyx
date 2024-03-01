# cython: c_string_type=str, c_string_encoding=utf8

from libcpp.string cimport string as std_string
from util.generic.string cimport std::string, npos

import pytest
import unittest

import sys


class TestStroka(unittest.TestCase):
    def test_unicode(self):
        cdef std::string x = "привет"
        self.assertEqual(x, "привет")


    def test_ctor1(self):
        cdef std::string tmp = std::string()
        cdef std::string tmp2 = std::string(tmp)
        self.assertEqual(tmp2, "")

    def test_ctor2(self):
        cdef std_string tmp = b"hello"
        cdef std::string tmp2 = std::string(tmp)
        self.assertEqual(tmp2, "hello")

    def test_ctor3(self):
        cdef std::string tmp = b"hello"
        cdef std::string tmp2 = std::string(tmp, 0, 4)
        self.assertEqual(tmp2, "hell")

    def test_ctor4(self):
        cdef std::string tmp = std::string(<char*>b"hello")
        self.assertEqual(tmp, "hello")

    def test_ctor5(self):
        cdef std::string tmp = std::string(<char*>b"hello", 4)
        self.assertEqual(tmp, "hell")

    def test_ctor6(self):
        cdef std::string tmp = std::string(<char*>b"hello", 1, 3)
        self.assertEqual(tmp, "ell")

    def test_ctor7(self):
        cdef std::string tmp = std::string(3, <char>'x')
        self.assertEqual(tmp, "xxx")

    def test_ctor8(self):
        cdef bytes tmp = b"hello"
        cdef std::string tmp2 = std::string(<char*>tmp, <char*>tmp + 4)
        self.assertEqual(tmp2, "hell")

    def test_compare(self):
        cdef std::string tmp1 = b"abacab"
        cdef std::string tmp2 = b"abacab"
        cdef std::string tmp3 = b"abacac"

        self.assertTrue(tmp1.compare(tmp2) == 0)
        self.assertTrue(tmp1.compare(tmp3) < 0)
        self.assertTrue(tmp3.compare(tmp1) > 0)

        self.assertTrue(tmp1 == tmp2)
        self.assertTrue(tmp1 != tmp3)

        self.assertTrue(tmp1 < tmp3)
        self.assertTrue(tmp1 <= tmp3)

        self.assertTrue(tmp3 > tmp1)
        self.assertTrue(tmp3 >= tmp1)

    def test_operator_assign(self):
        cdef std::string tmp = b"hello"
        cdef std::string tmp2 = tmp
        self.assertEqual(tmp2, "hello")

    def test_operator_plus(self):
        cdef std::string tmp = std::string(b"hello ") + std::string(b"world")
        self.assertEqual(tmp, "hello world")

    def test_c_str(self):
        cdef std::string tmp = b"hello"
        if sys.version_info.major == 2:
            self.assertEqual(bytes(tmp.c_str()), b"hello")
        else:
            self.assertEqual(bytes(tmp.c_str(), 'utf8'), b"hello")

    def test_length(self):
        cdef std::string tmp = b"hello"
        self.assertEqual(tmp.size(), tmp.length())

    def test_index(self):
        cdef std::string tmp = b"hello"

        self.assertEqual(<bytes>tmp[0], b'h')
        self.assertEqual(<bytes>tmp.at(0), b'h')

        self.assertEqual(<bytes>tmp[4], b'o')
        self.assertEqual(<bytes>tmp.at(4), b'o')

        # Actually, TString::at() is noexcept
        # with pytest.raises(IndexError):
        #     tmp.at(100)

    def test_append(self):
        cdef std::string tmp
        cdef std::string tmp2 = b"fuu"

        tmp.append(tmp2)
        self.assertEqual(tmp, "fuu")

        tmp.append(tmp2, 1, 2)
        self.assertEqual(tmp, "fuuuu")

        tmp.append(<char*>"ll ")
        self.assertEqual(tmp, "fuuuull ")

        tmp.append(<char*>"of greatness", 4)
        self.assertEqual(tmp, "fuuuull of g")

        tmp.append(2, <char>b'o')
        self.assertEqual(tmp, "fuuuull of goo")

        tmp.push_back(b'z')
        self.assertEqual(tmp, "fuuuull of gooz")

    def test_assign(self):
        cdef std::string tmp

        tmp.assign(b"one")
        self.assertEqual(tmp, "one")

        tmp.assign(b"two hundred", 0, 3)
        self.assertEqual(tmp, "two")

        tmp.assign(<char*>b"three")
        self.assertEqual(tmp, "three")

        tmp.assign(<char*>b"three fiddy", 5)
        self.assertEqual(tmp, "three")

    def test_insert(self):
        cdef std::string tmp

        tmp = b"xx"
        tmp.insert(1, b"foo")
        self.assertEqual(tmp, "xfoox")

        tmp = b"xx"
        tmp.insert(1, b"haxor", 1, 3)
        self.assertEqual(tmp, "xaxox")

        tmp = b"xx"
        tmp.insert(1, <char*>b"foo")
        self.assertEqual(tmp, "xfoox")

        tmp = b"xx"
        tmp.insert(1, <char*>b"foozzy", 3)
        self.assertEqual(tmp, "xfoox")

        tmp = b"xx"
        tmp.insert(1, 2, <char>b'u')
        self.assertEqual(tmp, "xuux")

    def test_copy(self):
        cdef char buf[16]
        cdef std::string tmp = b"hello"
        tmp.copy(buf, 5, 0)
        self.assertEqual(buf[:5], "hello")

    def test_find(self):
        cdef std::string haystack = b"whole lotta bytes"
        cdef std::string needle = "hole"

        self.assertEqual(haystack.find(needle), 1)
        self.assertEqual(haystack.find(needle, 3), npos)

        self.assertEqual(haystack.find(<char>b'h'), 1)
        self.assertEqual(haystack.find(<char>b'h', 3), npos)

    def test_rfind(self):
        cdef std::string haystack = b"whole lotta bytes"
        cdef std::string needle = b"hole"

        self.assertEqual(haystack.rfind(needle), 1)
        self.assertEqual(haystack.rfind(needle, 0), npos)

        self.assertEqual(haystack.rfind(<char>b'h'), 1)
        self.assertEqual(haystack.rfind(<char>b'h', 0), npos)

    def test_find_first_of(self):
        cdef std::string haystack = b"whole lotta bytes"
        cdef std::string cset = b"hxz"

        self.assertEqual(haystack.find_first_of(<char>b'h'), 1)
        self.assertEqual(haystack.find_first_of(<char>b'h', 3), npos)

        self.assertEqual(haystack.find_first_of(cset), 1)
        self.assertEqual(haystack.find_first_of(cset, 3), npos)

    def test_first_not_of(self):
        cdef std::string haystack = b"whole lotta bytes"
        cdef std::string cset = b"wxz"

        self.assertEqual(haystack.find_first_not_of(<char>b'w'), 1)
        self.assertEqual(haystack.find_first_not_of(<char>b'w', 3), 3)

        self.assertEqual(haystack.find_first_not_of(cset), 1)
        self.assertEqual(haystack.find_first_not_of(cset, 3), 3)

    def test_find_last_of(self):
        cdef std::string haystack = b"whole lotta bytes"
        cdef std::string cset = b"hxz"

        self.assertEqual(haystack.find_last_of(<char>b'h'), 1)
        self.assertEqual(haystack.find_last_of(<char>b'h', 0), npos)

        self.assertEqual(haystack.find_last_of(cset), 1)
        self.assertEqual(haystack.find_last_of(cset, 0), npos)

    def test_substr(self):
        cdef std::string tmp = b"foobar"

        self.assertEqual(tmp.substr(1), "oobar")
        self.assertEqual(tmp.substr(1, 4), "ooba")
