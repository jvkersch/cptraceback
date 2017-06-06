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
