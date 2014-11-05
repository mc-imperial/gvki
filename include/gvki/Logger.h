#ifndef SHADOW_CONTEXT_H
#define SHADOW_CONTEXT_H
#include "gvki/opencl_header.h"
#include <map>
#include <string>
#include <vector>
#include <fstream>

namespace gvki
{

struct BufferInfo
{
    size_t size;
    void* data;
    cl_mem_flags flags;
    BufferInfo() : size(0), data(0), flags(0) {}

    enum BufferTy
    {
        MEMORYBUFFER,
        IMAGE2D,
        IMAGE3D
    };
    BufferTy bufferType;
};

struct ProgramInfo
{
    std::vector<std::string> sources;
    std::string compileFlags;
};

struct ArgInfo
{
    const void* argValue;
    size_t argSize;
    ArgInfo() : argValue(0), argSize(0) { }
};

struct KernelInfo
{
    cl_program program;
    std::string entryPointName;
    std::vector<ArgInfo> arguments;
    size_t dimensions;
    std::vector<size_t> globalWorkOffset;
    std::vector<size_t> globalWorkSize;
    std::vector<size_t> localWorkSize;
};

class Logger
{
    public:
        std::map<cl_mem, BufferInfo> buffers;
        std::map<cl_program, ProgramInfo> programs;
        std::map<cl_kernel, KernelInfo> kernels;
        std::string directory;
        void openLog();
        void closeLog();

        Logger();
        ~Logger();
        void dump(cl_kernel k);

        static Logger& Singleton();
    private:
        // FIXME: Use std::unique_ptr<> instead
        std::ofstream* output;
        Logger(const Logger& that); /* = delete; */

        void printJSONArray(std::vector<size_t>& array);
        void printJSONKernelArgumentInfo(ArgInfo& ai);
        std::string dumpKernelSource(KernelInfo& ki);


};

}
#endif
