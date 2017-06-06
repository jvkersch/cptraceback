from other_pymod import fun

cdef class MyClass:

    cdef my_cdef_function(self):
        fun()

    def my_def_function(self):
        self.my_cdef_function()


def my_cython_fun():
    a = MyClass()
    a.my_def_function()
