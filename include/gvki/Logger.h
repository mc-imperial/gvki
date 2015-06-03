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
};

struct ImageInfo
{
   // TODO: Add more fields describing the image
   cl_mem_flags flags;
   cl_mem_object_type type;
};

struct SamplerInfo
{
    cl_bool normalized_coords;
    cl_addressing_mode addressing_mode;
    cl_filter_mode filter_mode;
};

// Information about where in the host code the "something" was created
struct HostAPICallInfo
{
    const char* compilationUnit;
    unsigned int lineNumber;
    const char* hostCodeFunctionCalled;

    HostAPICallInfo() : compilationUnit(0), lineNumber(0), hostCodeFunctionCalled(0) { }

    bool hasHostCodeInfo() const
    {
        return compilationUnit != 0 && hostCodeFunctionCalled !=0 && lineNumber > 0;
    }
};

struct ProgramInfo : public HostAPICallInfo
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

struct KernelInfo : public HostAPICallInfo
{
    cl_program program;
    std::string entryPointName;
    std::vector<ArgInfo> arguments;
    size_t dimensions;
    std::vector<size_t> globalWorkOffset;
    std::vector<size_t> globalWorkSize;
    std::vector<size_t> localWorkSize;
    bool localWorkSizeIsUnconstrained;
};

struct ProgramInfoCacheCompare
{
      bool operator() (const ProgramInfo& lhs, const ProgramInfo& rhs) const;
};

typedef std::map<ProgramInfo, std::string, ProgramInfoCacheCompare> ProgCacheMapTy;
class Logger
{
    public:
        std::map<cl_mem, BufferInfo> buffers;
        std::map<cl_mem, ImageInfo> images;
        std::map<cl_sampler, SamplerInfo> samplers;
        std::map<cl_program, ProgramInfo> programs;
        std::map<cl_kernel, KernelInfo> kernels;
        std::string directory;
        void openLog();
        void closeLog();

        Logger();
        ~Logger();
        void dump(cl_kernel k);
        BufferInfo * tryGetBuffer(ArgInfo &ai);

        static Logger& Singleton();
    private:
        // FIXME: Use std::unique_ptr<> instead
        std::ofstream* output;
        unsigned arrayDataCounter;
        Logger(const Logger& that); /* = delete; */
        void initDirectoryNumbered();
        void initDirectoryManual(const char* rootDir);

        void printJSONArray(std::vector<size_t>& array);
        void printJSONKernelArgumentInfo(ArgInfo& ai);
        void printJSONHostCodeInvocationInfo(HostAPICallInfo& info);
        std::string dumpKernelSource(KernelInfo& ki);

        ProgCacheMapTy WrittenKernelFileCache;
};

}
#endif
