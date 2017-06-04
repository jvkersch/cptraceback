#ifndef BACKTRACK_H
#define BACKTRACK_H

#include <string>
#include <vector>


class PyStackFrame
{
public:
    std::string filename;
    std::string funcname;
    int line;

    PyStackFrame(std::string filename, std::string funcname, int line)
        : filename(filename), funcname(funcname), line(line) {}
};


class CStackFrame
{
public:
    uint64_t pc;
    uint64_t offset;
    std::string funcname;
    PyStackFrame *pyFrame;

    CStackFrame(uint64_t pc, uint64_t offset, const std::string& funcname)
        : pc(pc), offset(offset), funcname(funcname), pyFrame(NULL) {}
};


class Traceback
{
public:
    Traceback();

    const std::vector<CStackFrame>& getStackFrames() const {
        return cFrames;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Traceback& tb);

private:
    std::vector<CStackFrame> cFrames;
    std::vector<PyStackFrame> pyFrames;
};


#endif // BACKTRACK_H
