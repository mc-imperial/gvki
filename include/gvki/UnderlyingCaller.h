#ifndef UNDERLYING_CALLER_H
#define UNDERLYING_CALLER_H

#include "gvki/opencl_header.h"

namespace gvki
{

class UnderlyingCaller
{
    public:
        typedef cl_mem (*clCreateBufferTy)(cl_context,
                                           cl_mem_flags,
                                           size_t,
                                           void*,
                                           cl_int*);
        clCreateBufferTy clCreateBufferU;

        typedef cl_mem (*clCreateSubBufferTy)(cl_mem,
                                              cl_mem_flags,
                                              cl_buffer_create_type,
                                              const void*,
                                              cl_int*
                                             );
        clCreateSubBufferTy clCreateSubBufferU;

        typedef cl_mem (*clCreateImage2DTy)(cl_context,
                                            cl_mem_flags,
                                            const cl_image_format*,
                                            size_t,
                                            size_t,
                                            size_t,
                                            void*,
                                            cl_int*
                                           );
        clCreateImage2DTy clCreateImage2DU;

        typedef cl_mem (*clCreateImage3DTy)(cl_context,
                                            cl_mem_flags,
                                            const cl_image_format*,
                                            size_t,
                                            size_t,
                                            size_t,
                                            size_t,
                                            size_t,
                                            void*,
                                            cl_int*
                                           );
        clCreateImage3DTy clCreateImage3DU;

#ifdef CL_VERSION_1_2
       typedef cl_mem (*clCreateImageTy)(cl_context context,
                                         cl_mem_flags flags,
                                         const cl_image_format* image_format,
                                         const cl_image_desc* image_desc,
                                         void* host_ptr,
                                         cl_int*  errcode_ret
                                        );
       clCreateImageTy clCreateImageU;
#endif

        typedef cl_sampler (*clCreateSamplerTy)(cl_context,
                                                cl_bool,
                                                cl_addressing_mode,
                                                cl_filter_mode,
                                                cl_int*
                                               );
        clCreateSamplerTy clCreateSamplerU;

        typedef cl_program (*clCreateProgramWithSourceTy)(cl_context,
                                                          cl_uint,
                                                          const char**,
                                                          const size_t*,
                                                          cl_int*);
        clCreateProgramWithSourceTy clCreateProgramWithSourceU;

        typedef cl_int (*clBuildProgramTy)(cl_program,
                                           cl_uint,
                                           const cl_device_id*,
                                           const char*,
                                           void (CL_CALLBACK *  /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
                                           void*);
        clBuildProgramTy clBuildProgramU;

        typedef cl_kernel (*clCreateKernelTy)(cl_program,
                                              const char*,
                                              cl_int*
                                             );
        clCreateKernelTy clCreateKernelU;

        typedef cl_int (*clCreateKernelsInProgramTy)(cl_program,
                                                     cl_uint,
                                                     cl_kernel*,
                                                     cl_uint*
                                                    );

        clCreateKernelsInProgramTy clCreateKernelsInProgramU;


        typedef cl_int (*clSetKernelArgTy)(cl_kernel,
                                           cl_uint,
                                           size_t,
                                           const void*
                                          );
        clSetKernelArgTy clSetKernelArgU;

        typedef cl_int(*clEnqueueNDRangeKernelTy)(cl_command_queue,
                                                  cl_kernel,
                                                  cl_uint,
                                                  const size_t*,
                                                  const size_t*,
                                                  const size_t*,
                                                  cl_uint,
                                                  const cl_event*,
                                                  cl_event*
                                                 );
        clEnqueueNDRangeKernelTy clEnqueueNDRangeKernelU;

        UnderlyingCaller();

        static UnderlyingCaller& Singleton();
};

}

#endif
