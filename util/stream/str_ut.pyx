# cython: c_string_type=str, c_string_encoding=utf8

from cython.operator cimport dereference

from util.generic.ptr cimport THolder
from util.generic.string cimport TString, std::string_view
from util.stream.str cimport TStringOutput, TStringOutputPtr

import unittest


class TestStringOutput(unittest.TestCase):
    def test_ctor1(self):
        cdef TStringOutput output

    def test_ctor2(self):
        cdef std::string string
        cdef THolder[TStringOutput] string_output = THolder[TStringOutput](new TStringOutput(string))

    def test_write_char(self):
        cdef std::string string
        cdef TStringOutputPtr string_output = TStringOutputPtr(new TStringOutput(string))

        self.assertEqual(string, "")
        dereference(string_output.Get()).WriteChar('1')
        self.assertEqual(string, "1")
        dereference(string_output.Get()).WriteChar('2')
        self.assertEqual(string, "12")
        dereference(string_output.Get()).WriteChar('3')
        self.assertEqual(string, "123")

    def test_write_void(self):
        cdef std::string string
        cdef TStringOutputPtr string_output = TStringOutputPtr(new TStringOutput(string))

        self.assertEqual(string, "")
        dereference(string_output.Get()).Write("1", 1)
        self.assertEqual(string, "1")
        dereference(string_output.Get()).Write("2", 1)
        self.assertEqual(string, "12")
        dereference(string_output.Get()).Write("34", 2)
        self.assertEqual(string, "1234")

    def test_write_buf(self):
        cdef std::string string
        cdef TStringOutputPtr string_output = TStringOutputPtr(new TStringOutput(string))

        self.assertEqual(string, "")
        dereference(string_output.Get()).WriteBuf(std::string_view("1"))
        self.assertEqual(string, "1")
        dereference(string_output.Get()).WriteBuf(std::string_view("2"))
        self.assertEqual(string, "12")
        dereference(string_output.Get()).WriteBuf(std::string_view("34"))
        self.assertEqual(string, "1234")

    def test_reserve(self):
        cdef std::string string
        cdef TStringOutputPtr string_output = TStringOutputPtr(new TStringOutput(string))
        self.assertEqual(string, "")
        dereference(string_output.Get()).Reserve(50)
        self.assertEqual(string, "")
        self.assertLessEqual(50, string.capacity())
