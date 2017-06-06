#include <Python.h>
#include "cbacktrace.h"

static PyObject *
call_print_tb(PyObject *self, PyObject *args)
{
    print_tb();

    Py_INCREF(Py_None);
    return Py_None;
}


static PyMethodDef TracebackMethods[] = {
    {"print_tb",  call_print_tb, METH_VARARGS, "Print traceback."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef tbmodule = {
   PyModuleDef_HEAD_INIT,
   "tb",
   NULL,
   -1,
   TracebackMethods
};

PyMODINIT_FUNC
PyInit_tb(void)
{
    return PyModule_Create(&tbmodule);
}
