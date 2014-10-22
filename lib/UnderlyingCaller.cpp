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
#define SET_FCN_PTR(functionName) dlerror(); /* clear any old errors */ \
                                  this->functionName ## U = reinterpret_cast<functionName ## Ty>(::dlsym(RTLD_NEXT, #functionName)); \
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

    // Initialise the function pointers
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

UnderlyingCaller& UnderlyingCaller::Singleton()
{
    static UnderlyingCaller uc;
    return uc;
}
