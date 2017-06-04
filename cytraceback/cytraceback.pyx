from cython.operator cimport dereference as deref
from libcpp.vector cimport vector
from libcpp.string cimport string
from libc.stdint cimport uint64_t


cdef extern from "backtrace.h":
    cppclass Traceback:
        Traceback()


cdef extern from "<iostream>" namespace "std":
    cdef cppclass ostream:
        ostream &operator<<(Traceback)
    ostream cerr


def print_tb():
    """ Print a mixed C/Python traceback to stderr.
    """
    cerr << Traceback()
