#include "gvki/Logger.h"
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <errno.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "string.h"
#include "gvki/Debug.h"


// For mkdir(). FIXME: Make windows compatible
#include <sys/stat.h>
// For opendir(). FIXME: Make windows compatible
#include <dirent.h>

// For getcwd()
#include <unistd.h>
#define FILE_SEP "/"

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
    int count = 0;
    bool success= false;

    // FIXME: Reading from the environment probably doesn't belong in here
    // but it makes implementing the singleton a lot easier
    std::string directoryPrefix;

    const char* envTemp = getenv("GVKI_ROOT");
    if (envTemp)
    {
        DEBUG_MSG("Using GVKI_ROOT value as destination for directories");
        DIR* dh = opendir(envTemp);
        if (dh == NULL)
        {
            ERROR_MSG(strerror(errno) << ". Directory was :" << envTemp);
            exit(1);
        }
        else
            closedir(dh);

        directoryPrefix = envTemp;

    }
    else
    {
        // Use the current working directory
        DEBUG_MSG("Using current working directory as destination for directories");

        // FIXME: Hard-coding this size is gross
        char cwdArray[1024];
        char* cwdResult = getcwd(cwdArray, sizeof(cwdArray)/sizeof(char));
        if (!cwdResult)
        {
            ERROR_MSG(strerror(errno) << ". Could not read the current working directory");
            exit(1);
        }
        else
            directoryPrefix = cwdResult;

    }
    directoryPrefix += FILE_SEP "gvki";
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
        if (mkdir(ss.str().c_str(), 0770) != 0)
        {
            if (errno != EEXIST)
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

    openLog();
}


void Logger::openLog()
{
    // FIXME: We should use mkstemp() or something
    std::stringstream ss;
    ss << directory << FILE_SEP << "log.json";
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
    for (int index=0; index < ki.globalWorkOffset.size() ; ++index)
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

    *output << "\"local_size\": ";
    printJSONArray(ki.localWorkSize);
    *output << "," << endl;

    assert(ki.globalWorkOffset.size() == ki.globalWorkSize.size() == ki.localWorkSize.size() &&
            "dimension mismatch");

    *output << "\"entry_point\": \"" << ki.entryPointName << "\"";

    // entry_point might be the last entry is there were no kernel args
    if (ki.arguments.size() == 0)
        *output << endl;
    else
    {
        *output << "," << endl << "\"kernel_arguments\": [ " << endl;
        for (int argIndex=0; argIndex < ki.arguments.size() ; ++argIndex)
        {
            printJSONKernelArgumentInfo(ki.arguments[argIndex]);
            if (argIndex != (ki.arguments.size() -1))
                *output << "," << endl;
        }
        *output << endl << "]" << endl;
    }


    *output << "}";
}

void Logger::printJSONArray(std::vector<size_t>& array)
{
    *output << "[ ";
    for (int index=0; index < array.size(); ++index)
    {
        *output << array[index];

        if (index != (array.size() -1))
            *output << ", ";
    }
    *output << "]";
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

    DEBUG_MSG("Note sizeof(cl_mem) == " << sizeof(cl_mem));
    // Hack:
    // It's hard to determine what type the argument is.
    // We can't dereference the void* to check if
    // it points to cl_mem we previously stored
    //
    // In some implementations cl_mem will be a pointer
    // which poses a risk if a scalar parameter of the same size
    // as the pointer type.
    if (ai.argSize == sizeof(cl_mem))
    {
       cl_mem mightBecl_mem = *((cl_mem*) ai.argValue);

       // We might be reading invalid data now
       if (buffers.count(mightBecl_mem) == 1)
       {
           // We're going to assume it's cl_mem that we saw before
           *output << "\"type\": \"array\",";
           BufferInfo& bi = buffers[mightBecl_mem];

           *output << "\"size\": " << bi.size << "}";
           return;
       }

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

        std::string withDir = (directory + FILE_SEP) + ss.str();
        if (!file_exists(withDir))
        {
           kos = new std::ofstream(withDir.c_str());

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
