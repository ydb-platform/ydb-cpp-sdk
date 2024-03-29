# cython: c_string_type=str, c_string_encoding=utf8

from util.generic.hash cimport THashMap

import pytest
import unittest

from libcpp.pair cimport pair
from cython.operator cimport dereference as deref


def _check_convert(THashMap[std::string, int] x):
    return x


class TestHash(unittest.TestCase):

    def test_constructors_and_assignments(self):
        cdef THashMap[std::string, int] c1
        c1["one"] = 1
        c1["two"] = 2
        cdef THashMap[std::string, int] c2 = THashMap[std::string, int](c1)
        self.assertEqual(2, c1.size())
        self.assertEqual(2, c2.size())
        self.assertEqual(1, c1.at("one"))
        self.assertTrue(c1.contains("two"))
        self.assertTrue(c2.contains("one"))
        self.assertEqual(2, c2.at("two"))
        c2["three"] = 3
        c1 = c2
        self.assertEqual(3, c1.size())
        self.assertEqual(3, c2.size())
        self.assertEqual(3, c1.at("three"))

    def test_equality_operator(self):
        cdef THashMap[std::string, int] base
        base["one"] = 1
        base["two"] = 2

        cdef THashMap[std::string, int] c1 = THashMap[std::string, int](base)
        self.assertTrue(c1==base)

        cdef THashMap[std::string, int] c2
        c2["one"] = 1
        c2["two"] = 2
        self.assertTrue(c2 == base)

        c2["three"] = 3
        self.assertTrue(c2 != base)

        cdef THashMap[std::string, int] c3 = THashMap[std::string, int](base)
        c3["one"] = 0
        self.assertTrue(c3 != base)

    def test_insert_erase(self):
        cdef THashMap[std::string, int] tmp
        self.assertTrue(tmp.insert(pair[std::string, int]("one", 0)).second)
        self.assertFalse(tmp.insert(pair[std::string, int]("one", 1)).second)
        self.assertTrue(tmp.insert(pair[std::string, int]("two", 2)).second)
        cdef std::string one = "one"
        cdef std::string two = "two"
        self.assertEqual(tmp.erase(one), 1)
        self.assertEqual(tmp.erase(two), 1)
        self.assertEqual(tmp.size(), 0)
        self.assertTrue(tmp.empty())

    def test_iterators_and_find(self):
        cdef THashMap[std::string, int] tmp
        self.assertTrue(tmp.begin() == tmp.end())
        self.assertTrue(tmp.find("1") == tmp.end())
        tmp["1"] = 1
        self.assertTrue(tmp.begin() != tmp.end())
        cdef THashMap[std::string, int].iterator it = tmp.find("1")
        self.assertTrue(it != tmp.end())
        self.assertEqual(deref(it).second, 1)

    def test_convert(self):
        src = {'foo': 1, 'bar': 42}
        self.assertEqual(_check_convert(src), src)

        bad_src = {'foo': 1, 'bar': 'baz'}
        with self.assertRaises(TypeError):
            _check_convert(bad_src)
