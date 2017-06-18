#ifndef MY_BACKTRACE_H
#define MY_BACKTRACE_H
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <Python.h>
#include <frameobject.h>

#ifdef _MSC_VER
#include <Windows.h>
#include <Dbghelp.h>
#else
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#undef UNW_LOCAL_ONLY
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Keep everything on the stack to avoid mallocing and freeing. All strings are
   char arrays of this length. */
#define BUF_LEN 256

/* Forward declarations */
struct c_frame;

/* Public API */
typedef void (*tb_formatter)(FILE *, struct c_frame *);
void dump_traceback(FILE *f, tb_formatter writer);
void print_tb();
void print_tb_to_file(FILE *f);

struct py_frame
{
    char filename[BUF_LEN];
    char funcname[BUF_LEN];
    size_t line;
};

struct c_frame
{
    uint64_t pc;
    uint64_t offset;
    char funcname[BUF_LEN];
    struct py_frame *pyframe;
};


/* Private API */

#define S(arr, idx, ch) (arr)[sizeof(arr) + idx] = ch
#define TRUNCATE(arr) \
    S(arr, -1, 0); S(arr, -2, '.'); S(arr, -3, '.'); S(arr, -4, '.');

#ifdef _MSC_VER
struct c_stack_ctx
{
	void *frames[1024];
	USHORT nFramesCaptured;
	USHORT currentFrame;
	HANDLE process;
};
#else
struct c_stack_ctx
{
    unw_cursor_t cursor;
    unw_context_t context;
};
#endif

struct py_stack_ctx
{
    PyThreadState *tstate;
    PyFrameObject *frame;
};

#ifdef _MSC_VER
static inline
void init_stack_ctx(struct c_stack_ctx *ctx)
{
	USHORT nFramesCaptured = CaptureStackBackTrace(
		0, 1024, ctx->frames, NULL
	);
	ctx->nFramesCaptured = nFramesCaptured;
	ctx->currentFrame = 0;

	HANDLE process = GetCurrentProcess();
	SymInitialize(process, NULL, TRUE);
	ctx->process = process;
}
#else
static inline
void init_stack_ctx(struct c_stack_ctx *ctx)
{
    // TODO check for error

    // Initialize cursor to current frame for local unwinding.
    unw_getcontext(&(ctx->context));
    unw_init_local(&(ctx->cursor), &(ctx->context));
}
#endif

static inline
int init_py_stack_ctx(struct py_stack_ctx *ctx)
{
    PyThreadState *tstate = PyThreadState_GET();

    if ((tstate == NULL) || (tstate->frame == NULL))
        return -1;

    ctx->tstate = tstate;
    ctx->frame = tstate->frame;

    return 0;
}

#ifdef _MSC_VER
static inline
int get_next_frame(struct c_stack_ctx *ctx, struct c_frame *frame)
{
	if (ctx->currentFrame >= ctx->nFramesCaptured)
		return -1;

	char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbol->MaxNameLen = MAX_SYM_NAME;

	IMAGEHLP_LINE64 line;
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	DWORD64 address = (DWORD64)ctx->frames[ctx->currentFrame];
	DWORD disp = 0;

	frame->pyframe = NULL;
	if (SymFromAddr(ctx->process, address, NULL, pSymbol)) {
		if (SymGetLineFromAddr64(ctx->process, address, &disp, &line)) {
			frame->pc = pSymbol->Address;
			frame->offset = line.LineNumber;
			strncpy(frame->funcname, pSymbol->Name, BUF_LEN);
			TRUNCATE(frame->funcname);
		} else {
			frame->pc = address;
			frame->offset = 0;
			strcpy(frame->funcname, "no debug info");
		}
	} else {
		//DWORD error = GetLastError();
	    //fprintf(stderr, "SymFromAddr returned an error: %d\n", error);
		frame->pc = address;
		frame->offset = 0;
		strcpy(frame->funcname, "invalid address");
	}

	ctx->currentFrame++;
	return 1;
}
#else
static inline
int get_next_frame(struct c_stack_ctx *ctx, struct c_frame *frame)
{
    unw_word_t pc, offset;

    if (unw_step(&ctx->cursor) <= 0)
        return -1;

    unw_get_reg(&ctx->cursor, UNW_REG_IP, &pc);
    if (pc == 0)
        return -1;

    if (unw_get_proc_name(&ctx->cursor, frame->funcname,
                          sizeof(frame->funcname), &offset) != 0)
        return -1;

    /* FIXME: need a test for long function names */

    frame->pc = pc;
    frame->offset = offset;
    frame->pyframe = NULL;

    TRUNCATE(frame->funcname);

    return 1;
}
#endif

static inline char*
_decode(PyObject* obj)
{
    /* TODO document whether this returns a pointer or a new string */

    /* TODO python3 specific */
    return PyUnicode_AsUTF8(obj);
}

static inline
int get_next_pyframe(struct py_stack_ctx *ctx, struct py_frame *pyframe)
{
    size_t line;
    char *filename, *funcname;
    PyFrameObject *frame = ctx->frame;

    if (frame == NULL)
        return -1;

    line = PyCode_Addr2Line(frame->f_code, frame->f_lasti);
    filename = _decode(frame->f_code->co_filename);
    funcname = _decode(frame->f_code->co_name);

    ctx->frame = frame->f_back;
    pyframe->line = line;
    strncpy(pyframe->filename, filename, sizeof(pyframe->filename));
    strncpy(pyframe->funcname, funcname, sizeof(pyframe->funcname));

    TRUNCATE(pyframe->filename);
    TRUNCATE(pyframe->funcname);

    return 0;
}

static inline
void file_tb_formatter(FILE *f, struct c_frame *frame)
{
    struct py_frame *pyframe = frame->pyframe;

    fprintf(f, "0x%llx: (%s+0x%llx)\n",
            frame->pc, frame->funcname, frame->offset);
    if (pyframe != NULL)
        fprintf(f, "\tPython: %s (%s:%zu)\n",
                pyframe->funcname, pyframe->filename, pyframe->line);
}

void dump_traceback(FILE *f, tb_formatter writer)
{
    struct c_stack_ctx ctx;
    struct py_stack_ctx py_ctx;
    struct c_frame frame;
    struct py_frame pyframe;

    init_stack_ctx(&ctx);
    init_py_stack_ctx(&py_ctx);

    // Unwind frames one by one, going up the frame stack.
    while (get_next_frame(&ctx, &frame) > 0) {

        if (strcmp(frame.funcname, "_PyEval_EvalFrameDefault") == 0) {
            if (get_next_pyframe(&py_ctx, &pyframe) == 0) {
                frame.pyframe = &pyframe;
            }
        }

        writer(f, &frame);
    }
}

void print_tb()
{
    dump_traceback(stderr, file_tb_formatter);
}

void print_tb_to_file(FILE *f)
{
    dump_traceback(f, file_tb_formatter);
}


#ifdef __cplusplus
}
#endif

#endif /* MY_BACKTRACE_H */
