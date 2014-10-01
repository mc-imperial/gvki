#include "gvki/UnderlyingCaller.h"
#include <iostream>
#include <cstdlib>

#ifndef MACRO_LIB
#include <dlfcn.h>
#endif

using namespace gvki;

#ifdef MACRO_LIB
#define SET_FCN_PTR(functionName) functionName ## U = &(::functionName);
#else
#include <unistd.h>
#define _SET_FCN_PTR(functionName, symbol) dlerror(); /* clear any old errors */ \
                       functionName ## U = reinterpret_cast<functionName ## Ty>(::dlsym(RTLD_NEXT, #symbol)); \
                       errorMsg = dlerror(); \
                       if ( errorMsg != NULL) { \
                         std::cerr << "Failed to dlsym(\"" #symbol  "\"): " << errorMsg << std::endl; \
                         _exit(255); \
                       } \
                       if ( functionName ## U == NULL) { \
                         std::cerr << "Function pointer for \"" #functionName "\" cannot be NULL" << std::endl; \
                         _exit(255); \
                       }

#ifdef __APPLE__
// Under OSX the OpenCL symbols are prefixed with an underscore [1] claims that
// dlsym() is supposed to handle that for us but testing by others seemed to
// suggest this wasn't the case.
//
// [1] https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man3/dlsym.3.html
#define SET_FCN_PTR(functionName) _SET_FCN_PTR(functionName, _ ## functionName)
#else
#define SET_FCN_PTR(functionName) _SET_FCN_PTR(functionName, functionName)
#endif


#endif

UnderlyingCaller::UnderlyingCaller()
{
    const char* errorMsg = 0;
    SET_FCN_PTR(clCreateBuffer)
    SET_FCN_PTR(clCreateSubBuffer)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    // In OpenCL 1.2 these are deprecated. Don't warn about this.
    SET_FCN_PTR(clCreateImage2D)
    SET_FCN_PTR(clCreateImage3D)
#pragma GCC diagnostic pop
    SET_FCN_PTR(clCreateSampler)
    SET_FCN_PTR(clCreateProgramWithSource)
    SET_FCN_PTR(clBuildProgram)
    SET_FCN_PTR(clCreateKernel)
    SET_FCN_PTR(clSetKernelArg)
    SET_FCN_PTR(clEnqueueNDRangeKernel)
};
