#include "gvki/Logger.h"
#include "gvki/PathSeperator.h"
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <errno.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "string.h"
#include "gvki/Debug.h"

#include <sys/stat.h>

#ifndef _WIN32

#include <dirent.h>
#include <unistd.h>
#define MKDIR_FAILS(d)     (mkdir(d, 0770) != 0)
#define DIR_ALREADY_EXISTS (errno == EEXIST)

static void checkDirectoryExists(const char* dirName) {
    DIR* dh = opendir(dirName);
    if (dh != NULL)
    {
        closedir(dh);
        return;
    }
    ERROR_MSG(strerror(errno) << ". Directory was :" << dirName);
    exit(1);
}

#else

#include <C:/prog/msinttypes/inttypes.h>
#include <Windows.h>
#define MKDIR_FAILS(d)     (CreateDirectory(d, NULL) == 0)
#define DIR_ALREADY_EXISTS (GetLastError() == ERROR_ALREADY_EXISTS)

static void checkDirectoryExists(const char* dirName) {
    DWORD ftyp = GetFileAttributesA(dirName);
    if ((ftyp != INVALID_FILE_ATTRIBUTES) && ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return;
    ERROR_MSG(strerror(errno) << ". Directory was :" << dirName);
    exit(1);
}

#endif

using namespace std;
using namespace gvki;

// Maximum number of files or directories
// that can be created
static const int maxFiles = 10000;

Logger& Logger::Singleton()
{
    static Logger l;
    return l;
}

Logger::Logger()
{
    // FIXME: Reading from the environment probably doesn't belong in here
    // but it makes implementing the singleton a lot easier
    const char* doNotUseNumberedDirs = getenv("GVKI_NO_NUM_DIRS");
    if (doNotUseNumberedDirs)
    {
        DEBUG_MSG("Using manual directory");
        const char* rootDir = getenv("GVKI_ROOT");
        if (!rootDir)
        {
            ERROR_MSG("If using GVKI_NO_NUM_DIRS then GVKI_ROOT must be specified");
            _exit(1);
        }
        initDirectoryManual(rootDir);
    }
    else
    {
        DEBUG_MSG("Using numbered directory");
        initDirectoryNumbered();
    }
    checkDirectoryExists(this->directory.c_str());
    DEBUG_MSG("Directory used for logging is \"" << this->directory << "\"");

    openLog();
}

void Logger::initDirectoryManual(const char* rootDir)
{
    assert(rootDir && "rootDir cannot be NULL");

    if (MKDIR_FAILS(rootDir))
    {
        ERROR_MSG("Failed to create directory \"" << rootDir << ": " << strerror(errno));
        _exit(1);
    }
    this->directory = std::string(rootDir);
}

void Logger::initDirectoryNumbered()
{
    int count = 0;
    bool success= false;

    std::string directoryPrefix;

    const char* envTemp = getenv("GVKI_ROOT");
    if (envTemp)
    {
        DEBUG_MSG("Using GVKI_ROOT value as destination for directories");

        checkDirectoryExists(envTemp);

        directoryPrefix = envTemp;

    }
    else
    {
        // Use the current working directory
        DEBUG_MSG("Using current working directory as destination for directories");

        // FIXME: Hard-coding this size is gross
        #define MAX_DIR_LENGTH 1024

#ifndef _WIN32
        char cwdArray[MAX_DIR_LENGTH];
        char* cwdResult = getcwd(cwdArray, sizeof(cwdArray)/sizeof(char));
        if (!cwdResult)
#else
        char cwdResult[MAX_DIR_LENGTH];
        if (GetCurrentDirectory(MAX_DIR_LENGTH, cwdResult) == 0)
#endif
        {
            ERROR_MSG(strerror(errno) << ". Could not read the current working directory");
            exit(1);
        }
        else
            directoryPrefix = cwdResult;
    }
    directoryPrefix += PATH_SEP "gvki";
    DEBUG_MSG("Directory prefix is \"" << directoryPrefix << "\"");

    // Keep trying a directoryPrefix name with a number as suffix
    // until we find an available one or we exhaust the maximum
    // allowed number
    while (count < maxFiles)
    {
        stringstream ss;
        ss <<  directoryPrefix << "-" << count;
        ++count;

        // Make the directoryPrefix
        if (MKDIR_FAILS(ss.str().c_str()))
        {
            if (!DIR_ALREADY_EXISTS)
            {
                ERROR_MSG(strerror(errno) << ". Directory was :" << directoryPrefix);
                exit(1);
            }

            // It already exists, try again
            continue;
        }

        this->directory = ss.str();
        success = true;
        break;
    }

    if (!success)
    {
        ERROR_MSG("Exhausted available directory names or couldn't create any");
        exit(1);
    }

}


void Logger::openLog()
{
    // FIXME: We should use mkstemp() or something
    std::stringstream ss;
    ss << directory << PATH_SEP << "log.json";
    output = new std::ofstream(ss.str().c_str(), std::ofstream::out | std::ofstream::ate);

    if (! output->good())
    {
        ERROR_MSG("Failed to create file (" << ss.str() <<  ") to write log to");
        exit(1);
    }

    // Start of JSON array
    *output << "[" << std::endl;
}

void Logger::closeLog()
{
    assert(output != NULL && "output must not be NULL");
    // End of JSON array
    *output << std::endl << "]" << std::endl;
    output->close();
}

Logger::~Logger()
{
    closeLog();
    delete output;
}

void Logger::dump(cl_kernel k)
{
    // Output JSON format defined by
    // http://multicore.doc.ic.ac.uk/tools/GPUVerify/docs/json_format.html
    KernelInfo& ki = kernels[k];
    assert( programs.count(ki.program) == 1 && "cl_program missing");
    ProgramInfo& pi = programs[ki.program];

    static bool isFirst = true;

    if (!isFirst)
    {
        // Emit array element seperator
        // to seperate from previous dump
        *output << "," << endl;
    }
    isFirst = false;

    *output << "{" << endl << "\"language\": \"OpenCL\"," << endl;

    std::string kernelSourceFile = dumpKernelSource(ki);

    *output << "\"kernel_file\": \"" << kernelSourceFile << "\"," << endl;

    // FIXME: Teach GPUVerify how to handle non zero global_offset
    // FIXME: Document this json attribute!
    // Only emit global_offset if its non zero
    bool hasNonZeroGlobalOffset = false;
    for (unsigned index=0; index < ki.globalWorkOffset.size() ; ++index)
    {
        if (ki.globalWorkOffset[index] != 0)
            hasNonZeroGlobalOffset = true;
    }
    if (hasNonZeroGlobalOffset)
    {
        *output << "\"global_offset\": ";
        printJSONArray(ki.globalWorkOffset);
        *output << "," << endl;
    }

    *output << "\"global_size\": ";
    printJSONArray(ki.globalWorkSize);
    *output << "," << endl;

    // Note if local_size is unconstrained
    // we just don't emit the ``local_size`` key or its value.
    if (!ki.localWorkSizeIsUnconstrained)
    {
        *output << "\"local_size\": ";
        printJSONArray(ki.localWorkSize);
        *output << "," << endl;
    }

    *output << "\"compiler_flags\": \"" << pi.compileFlags << "\"," << endl;

    assert( (ki.globalWorkOffset.size() == ki.globalWorkSize.size()) &&
            (ki.globalWorkSize.size() == ki.localWorkSize.size()) &&
            "dimension mismatch");

    // Emit information about host code API calls used to build the kernel
    // and enqueue it if available
    if (pi.hasHostCodeInfo() || ki.hasHostCodeInfo())
    {
        *output << "\"host_api_calls\": [" << endl;
        bool mightNeedComma = false;

        if (pi.hasHostCodeInfo())
        {
            printJSONHostCodeInvocationInfo(pi);
            mightNeedComma = true;
        }

        if (ki.hasHostCodeInfo())
        {
            if (mightNeedComma)
                *output << "," << endl;

            printJSONHostCodeInvocationInfo(ki);
        }

        *output << "]," << endl;
    }

    *output << "\"entry_point\": \"" << ki.entryPointName << "\"";


    // entry_point might be the last entry is there were no kernel args
    if (ki.arguments.size() == 0)
        *output << endl;
    else
    {
        *output << "," << endl << "\"kernel_arguments\": [" << endl;
        for (unsigned argIndex=0; argIndex < ki.arguments.size() ; ++argIndex)
        {
            printJSONKernelArgumentInfo(ki.arguments[argIndex]);
            if (argIndex != (ki.arguments.size() -1))
                *output << "," << endl;
        }
        *output << endl << "]" << endl;
    }


    *output << "}";
}

void Logger::printJSONHostCodeInvocationInfo(HostAPICallInfo& info)
{
    assert(info.hasHostCodeInfo() && "no host code info available");
    *output << "{" << endl << "\"function_name\": \"" << info.hostCodeFunctionCalled << "\"," << endl <<
               "\"compilation_unit\": \"" << info.compilationUnit << "\"," << endl <<
               "\"line_number\": " << info.lineNumber << endl << "}" << endl;
}

void Logger::printJSONArray(std::vector<size_t>& array)
{
    *output << "[";
    for (unsigned index=0; index < array.size(); ++index)
    {
        *output << array[index];

        if (index != (array.size() -1))
            *output << ", ";
    }
    *output << "]";
}

BufferInfo * Logger::tryGetBuffer(ArgInfo& ai) {

    // Hack:
    // It's hard to determine what type the argument is.
    // We can't dereference the void* to check if
    // it points to cl_mem we previously stored
    //
    // In some implementations cl_mem will be a pointer
    // which poses a risk if a scalar parameter of the same size
    // as the pointer type.

    if (ai.argSize != sizeof(cl_mem))
    {
        return NULL;
    }

    cl_mem mightBecl_mem = *((cl_mem*)ai.argValue);

    // We might be reading invalid data now
    if (buffers.count(mightBecl_mem) != 1)
    {
        return NULL;
    }

    // We're going to assume it's cl_mem that we saw before
    return &buffers[mightBecl_mem];
}

void Logger::printJSONKernelArgumentInfo(ArgInfo& ai)
{
    *output << "{";
    if (ai.argValue == NULL)
    {
        // NULL was passed to clSetKernelArg()
        // That implies its for unallocated memory
        *output << "\"type\": \"array\",";

        // If the arg is for local memory
        if (ai.argSize != sizeof(cl_mem) && ai.argSize != sizeof(cl_sampler))
        {
            // We assume this means this arguments is for local memory
            // where size actually means the sizeof the underlying buffer
            // rather than the size of the type.
            *output << "\"size\" : " << ai.argSize;
        }
        *output << "}";
        return;
    }

    // FIXME:
    // Eurgh... the spec says
    // ```
    // If the argument is a buffer object, the arg_value pointer can be NULL or
    // point to a NULL value in which case a NULL value will be used as the
    // value for the argument declared as a pointer to __global or __constant
    // memory in the kernel.  ```
    //
    // This makes it impossible (seeing as we don't know which address space
    // the arguments are in) to work out the argument type once derefencing the
    // pointer and finding it's equal zero because it could be a scalar constant
    // (of value 0) or it could be an unintialised array!

    if (BufferInfo * bi = tryGetBuffer(ai))
    {
        *output << "\"type\": \"array\", ";

        *output << "\"size\": " << bi->size << ", ";

        *output << "\"flags\": \"";
        switch (bi->flags)
        {
            case CL_MEM_READ_ONLY:
                *output << "CL_MEM_READ_ONLY";
                break;
            case CL_MEM_WRITE_ONLY:
                *output << "CL_MEM_WRITE_ONLY";
                break;
            case CL_MEM_READ_WRITE:
                *output << "CL_MEM_READ_WRITE";
                break;
            default:
                *output << "UNKNOWN";
        }
        *output << "\"";

        if (bi->data != NULL)
        {
            *output << ", \"data\": [";
            for (size_t i = 0; i < bi->size; ++i)
            {
                if (i > 0)
                {
                    *output << ",";
                }
                *output << static_cast<unsigned>(((unsigned char*)bi->data)[i]);
            }
            *output << "]";
        }

        *output << "}";

        return;

    }

    // Hack: a similar hack of the image types
    if (ai.argSize == sizeof(cl_mem))
    {
       cl_mem mightBecl_mem = *((cl_mem*) ai.argValue);

       // We might be reading invalid data now
       if (images.count(mightBecl_mem) == 1)
       {
           // We're going to assume it's cl_mem that we saw before
           *output << "\"type\": \"image\"}";
           return;
       }

    }

    // Hack: a similar hack for the samplers
    if (ai.argSize == sizeof(cl_sampler))
    {
       cl_sampler mightBecl_sampler = *((cl_sampler*) ai.argValue);

       // We might be reading invalid data now
       if (samplers.count(mightBecl_sampler) == 1)
       {
           // We're going to assume it's cl_mem that we saw before
           *output << "\"type\": \"sampler\"}";
           return;
       }

    }

    // I guess it's scalar???
    *output << "\"type\": \"scalar\",";
    *output << " \"value\": \"0x";
    // Print the value as hex
    uint8_t* asByte = (uint8_t*) ai.argValue;
    // We assume the host is little endian so to print the values
    // we need to go through the array bytes backwards
    for (int byteIndex= ai.argSize -1; byteIndex >=0 ; --byteIndex)
    {
       *output << std::hex << std::setfill('0') << std::setw(2) << ( (unsigned) asByte[byteIndex]);
    }
    *output << "\"" << std::dec; //std::hex is sticky so switch back to decimal

    *output << "}";
    return;

}

static bool file_exists(std::string& name) {
	ifstream f(name.c_str());
	bool result = f.good();
	f.close();
	return result;
}

// Define the strict weak ordering over ProgramInfo instances
// Is this correct?
bool ProgramInfoCacheCompare::operator() (const ProgramInfo& lhs, const ProgramInfo& rhs) const
{
    // Use the fact that a strict-weak order is already defined over std::vector<>
    return lhs.sources < rhs.sources;
}

std::string Logger::dumpKernelSource(KernelInfo& ki)
{
    ProgramInfo& pi = programs[ki.program];

    // See if we can used a file that we already printed.
    // This avoid writing duplicate files.
    ProgCacheMapTy::iterator it = WrittenKernelFileCache.find(pi);
    if ( it != WrittenKernelFileCache.end() )
    {
        return it->second;
    }


    int count = 0;
    bool success = false;
    // FIXME: I really want a std::unique_ptr
    std::ofstream* kos = 0;
    std::string theKernelPath;
    while (count < maxFiles)
    {
        stringstream ss;
        ss << ki.entryPointName << "." << count << ".cl";

        ++count;

        std::string withDir = (directory + PATH_SEP) + ss.str();
        if (!file_exists(withDir))
        {
           // Use Binary mode to try avoid line ending issues on Windows
           kos = new std::ofstream(withDir.c_str(), std::ofstream::binary);

           if (!kos->good())
           {
                kos->close();
                delete kos;
                continue;
           }

           success = true;
           theKernelPath = ss.str();
           break;
        }
    }

    if (!success)
    {
        ERROR_MSG("Failed to log kernel output");
        return std::string("FIXME");
    }

    // Write kernel source

    for (vector<string>::const_iterator b = pi.sources.begin(), e = pi.sources.end(); b != e; ++b)
    {
        *kos << *b;
    }

    // Urgh this is bad, need RAII!
    kos->close();
    delete kos;

    // Store in cache
    assert(WrittenKernelFileCache.count(pi) == 0 && "ProgramInfo already in cache!");
    WrittenKernelFileCache[pi] = theKernelPath;

    return theKernelPath;
}
