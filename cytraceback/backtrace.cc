#include <Python.h>
#include <frameobject.h>

#include <cxxabi.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include <iostream>
#include <cstdlib>

#include "backtrace.h"


static const char*
decode(PyObject* obj)
{
    return PyUnicode_AsUTF8(obj);
}


static std::string
demangle(const char* symname)
{
    int status;
    char* realname = abi::__cxa_demangle(symname, 0, 0, &status);
    std::string demangled = status == 0? realname : symname;
    std::free(realname);
    return demangled;
}


std::vector<PyStackFrame> getPyTraceback()
{
    PyThreadState *tstate;
    PyFrameObject *frame = NULL;

    std::vector<PyStackFrame> frames;

    tstate = PyThreadState_GET();
    if ((tstate != NULL) && (tstate->frame != NULL))
        frame = tstate->frame;

    while (frame != NULL) {
        int line = PyCode_Addr2Line(frame->f_code, frame->f_lasti);
        const char *filename = decode(frame->f_code->co_filename);
        const char *funcname = decode(frame->f_code->co_name);
        frame = frame->f_back;
        frames.push_back(PyStackFrame(filename, funcname, line));
    }

    return frames;
}


std::vector<CStackFrame> getCTraceback()
{
    std::vector<CStackFrame> frames;

    unw_cursor_t cursor;
    unw_context_t context;

    // Initialize cursor to current frame for local unwinding.
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    // Unwind frames one by one, going up the frame stack.
    while (unw_step(&cursor) > 0) {
        uint64_t pc, offset;

        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0) {
            break;
        }

        char sym[256];
        if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) != 0)
            break;
        
        sym[sizeof(sym) - 1] = 0;
        frames.push_back(CStackFrame(pc, offset, demangle(sym)));
    }

    return frames;
}


void interleaveStackFrames(
    std::vector<CStackFrame>& cFrames,
    std::vector<PyStackFrame>& pyFrames,
    const std::string frameEvalName)
{
    size_t j = 0;
    for (size_t i = 0; i < cFrames.size(); i++) {
        if ((cFrames[i].funcname == frameEvalName) && (j < pyFrames.size()))
            cFrames[i].pyFrame = &pyFrames[j++];
    }
}


Traceback::Traceback()
{
    // need to acquire the GIL here.
    
    cFrames = getCTraceback();
    pyFrames = getPyTraceback();

    // can release the GIL here
    
    interleaveStackFrames(cFrames, pyFrames, "_PyEval_EvalFrameDefault");
}


std::ostream& operator<<(std::ostream& os, const PyStackFrame& pyFrame)
{
    os << pyFrame.filename << "(" << pyFrame.line << "): "
       << pyFrame.funcname;
    return os;
}


std::ostream& operator<<(std::ostream& os, const CStackFrame& cFrame)
{
    os << std::hex << cFrame.pc << ": "
       << cFrame.funcname << "+" << cFrame.offset;
    PyStackFrame *p = cFrame.pyFrame;
    if (p != NULL)
        os << "\tPython: " << (*p);
    return os;
}


std::ostream& operator<<(std::ostream& os, const Traceback& tb)
{
    for (size_t i = 0; i < tb.cFrames.size(); i++) {
        os << tb.cFrames[i] << std::endl;
    }
    return os;
}
