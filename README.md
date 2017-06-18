CPTraceback
===========

[![Build Status](https://travis-ci.org/jvkersch/cptraceback.svg?branch=master)](https://travis-ci.org/jvkersch/cptraceback)

CPTraceback is a package to print mixed Python/C stack traces from any point in
Python or C program.

It has no external dependencies, and comes as a single header file with a tiny
Python wrapper (written in C) for ease of use. Work is underway to make
CPTraceback allocate no memory on the heap, so that it can safely be used
inside a signal handler.

_This package only works reliably on Linux/Mac OS with Python 3.6_. There is
some support for Windows, but it is experimental. Supporting other Python
versions is feasible, but not (yet) planned. Open an issue if either of those
points is relevant to you.

Usage
-----

Clone this repository, then run `pip install -e .` to build and install the
module. At the point where you want to generate a traceback, import the module
and call `print_tb`:
```python
    from cptraceback import print_tb
    print_tb()
```

This will dump a traceback of the form
```
0x1007ecd3a: (call_print_tb+0x1a)
0x10005a901: (_PyCFunction_FastCallDict+0x1f1)
0x1000dc3d7: (call_function+0x1b7)
0x1000d8bb2: (_PyEval_EvalFrameDefault+0x6ad2)
	Python: my_python_function (examples/python_calling_cython/other_pymod.py:3)
0x1007e730b: (__Pyx_PyFunction_FastCallDict+0x2eb)
0x1007e693c: (__pyx_f_5cymod_7MyClass_my_cdef_function+0x7c)
0x1007e75ba: (__pyx_pw_5cymod_7MyClass_1my_def_function+0xa)
(...)
```

Invocations of `_PyEval_EvalFrameDefault` will have information about the
Python function being invoked attached to them.

Generating a traceback from C
-----------------------------

This package can also be used directly from C. Just `#include "cptraceback.h"`
and call `print_tb()` whenever you want to generate a traceback. This will
write a traceback to stderr. To write to a file, use `print_tb_to_file(FILE*)`
or provide a custom formatter to `dump_traceback(FILE*, tb_formatter*)`.

Technical details
-----------------

Under Linux/Mac OS, this uses the libunwind API to generate a stack trace. This
is described very well in
a
[blog post by Eli Bendersky](http://eli.thegreenplace.net/2015/programmatic-access-to-the-call-stack-in-c/). In fact, the code here is adapted directly from his blog entry.

Under Windows, we use `CaptureStackBackTrace` to get a stack trace, and
the
[DbgHelp API](https://msdn.microsoft.com/en-us/library/windows/desktop/ms679309.aspx) to
get information about the stack frames. I'm having trouble identifying stack
frames that belong to the Python interpreter, so this is still experimental.

License
-------

This package is licensed under the 2-clause BSD license.
