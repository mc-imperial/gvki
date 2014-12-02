#include "gvki/UnderlyingCaller.h"
#include "gvki/Config.h"
#include <iostream>
#include <cstdlib>

using namespace gvki;

#ifdef MACRO_LIB
#define SET_FCN_PTR(functionName) functionName ## U = &(::functionName);
#else

#include <dlfcn.h>
#include <unistd.h>
#define SET_FCN_PTR(functionName) dlerror(); /* clear any old errors */ \
                                  this->functionName ## U = reinterpret_cast<functionName ## Ty>(::dlsym(OCL_HANDLE, #functionName)); \
                                  errorMsg = dlerror(); \
                                  if ( errorMsg != NULL) { \
                                      std::cerr << "Failed to dlsym(\"" #functionName  "\"): " << errorMsg << std::endl; \
                                      _exit(255); \
                                  } \
                                  if ( this->functionName ## U == NULL) { \
                                      std::cerr << "Function pointer for \"" #functionName "\" cannot be NULL" << std::endl; \
                                      _exit(255); \
                                  }

#endif

UnderlyingCaller::UnderlyingCaller()
{
    const char* errorMsg = 0;

#ifndef MACRO_LIB

#ifdef __APPLE__
    // I can't seem to get dlsym(RTLD_NEXT, ...) to work so we'll do this instead
    // on OSX
    void* handle = dlopen( OPENCL_LIBRARY_ABS_PATH, RTLD_NOW);
    if ( handle == NULL)
    {
        std::cerr << "Failed to call dlopen(" OPENCL_LIBRARY_ABS_PATH  ", RTLD_NOW)" << std::endl;
        errorMsg = dlerror();
        if (errorMsg != NULL)
        {
            std::cerr << errorMsg << std::endl;
        }
        _exit(255);
    }
    #define OCL_HANDLE handle
#else
    // On Linux using RTLD_NEXT as the handle seems to work fine.
    #define OCL_HANDLE RTLD_NEXT
#endif

#endif

    // Initialise the function pointers
    SET_FCN_PTR(clCreateBuffer)
    SET_FCN_PTR(clCreateSubBuffer)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    // In OpenCL 1.2 these are deprecated. Don't warn about this.
    SET_FCN_PTR(clCreateImage2D)
    SET_FCN_PTR(clCreateImage3D)
#pragma GCC diagnostic pop

#ifdef CL_VERSION_1_2
    SET_FCN_PTR(clCreateImage)
#endif

    SET_FCN_PTR(clCreateSampler)
    SET_FCN_PTR(clCreateProgramWithSource)
    SET_FCN_PTR(clBuildProgram)
    SET_FCN_PTR(clCreateKernel)
    SET_FCN_PTR(clCreateKernelsInProgram)
    SET_FCN_PTR(clSetKernelArg)
    SET_FCN_PTR(clEnqueueNDRangeKernel)
};

UnderlyingCaller& UnderlyingCaller::Singleton()
{
    static UnderlyingCaller uc;
    return uc;
}
