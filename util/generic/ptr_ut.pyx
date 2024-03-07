from libcpp.utility cimport pair
from util.generic.ptr cimport MakeAtomicShared, TAtomicSharedPtr, THolder
from util.system.types cimport ui64

import pytest
import unittest


class TestHolder(unittest.TestCase):

    def test_basic(self):
        cdef THolder[std::string] holder
        holder.Reset(new std::string("aaa"))
        assert holder.Get()[0] == "aaa"
        holder.Destroy()
        assert holder.Get() == NULL
        holder.Reset(new std::string("bbb"))
        assert holder.Get()[0] == "bbb"
        holder.Reset(new std::string("ccc"))
        assert holder.Get()[0] == "ccc"

    def test_make_atomic_shared(self):
        cdef TAtomicSharedPtr[pair[ui64, std::string]] atomic_shared_ptr = MakeAtomicShared[pair[ui64, std::string]](15, "Some string")
        assert atomic_shared_ptr.Get().first == 15
        assert atomic_shared_ptr.Get().second == "Some string"
