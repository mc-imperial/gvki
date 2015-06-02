#ifndef UNDERLYING_CALLER_H
#define UNDERLYING_CALLER_H

#include "gvki/opencl_header.h"

namespace gvki
{

// FIXME: Should we define our own macro rather than abusing
// CL_CALLBACK?
class UnderlyingCaller
{
    public:
        typedef cl_mem (CL_CALLBACK *clCreateBufferTy)(cl_context,
                                                       cl_mem_flags,
                                                       size_t,
                                                       void*,
                                                       cl_int*);
        clCreateBufferTy clCreateBufferU;

        typedef cl_mem (CL_CALLBACK *clCreateSubBufferTy)(cl_mem,
                                                          cl_mem_flags,
                                                          cl_buffer_create_type,
                                                          const void*,
                                                          cl_int*
                                                         );
        clCreateSubBufferTy clCreateSubBufferU;

        typedef cl_mem (CL_CALLBACK *clCreateImage2DTy)(cl_context,
                                                        cl_mem_flags,
                                                        const cl_image_format*,
                                                        size_t,
                                                        size_t,
                                                        size_t,
                                                        void*,
                                                        cl_int*
                                                       );
            clCreateImage2DTy clCreateImage2DU;

        typedef cl_mem (CL_CALLBACK *clCreateImage3DTy)(cl_context,
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
       typedef cl_mem (CL_CALLBACK *clCreateImageTy)(cl_context context,
                                                     cl_mem_flags flags,
                                                     const cl_image_format* image_format,
                                                     const cl_image_desc* image_desc,
                                                     void* host_ptr,
                                                     cl_int*  errcode_ret
                                                    );
       clCreateImageTy clCreateImageU;
#endif

        typedef cl_sampler (CL_CALLBACK *clCreateSamplerTy)(cl_context,
                                                            cl_bool,
                                                            cl_addressing_mode,
                                                            cl_filter_mode,
                                                            cl_int*
                                                           );
        clCreateSamplerTy clCreateSamplerU;

        typedef cl_program (CL_CALLBACK *clCreateProgramWithSourceTy)(cl_context,
                                                                      cl_uint,
                                                                      const char**,
                                                                      const size_t*,
                                                                      cl_int*);
        clCreateProgramWithSourceTy clCreateProgramWithSourceU;

        typedef cl_int (CL_CALLBACK *clBuildProgramTy)(cl_program,
                                                       cl_uint,
                                                       const cl_device_id*,
                                                       const char*,
                                                       void (CL_CALLBACK *  /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
                                                       void*);
        clBuildProgramTy clBuildProgramU;

        typedef cl_kernel (CL_CALLBACK *clCreateKernelTy)(cl_program,
                                                          const char*,
                                                          cl_int*
                                                         );
            clCreateKernelTy clCreateKernelU;

        typedef cl_int (CL_CALLBACK *clCreateKernelsInProgramTy)(cl_program,
                                                                 cl_uint,
                                                                 cl_kernel*,
                                                                 cl_uint*
                                                                );

        clCreateKernelsInProgramTy clCreateKernelsInProgramU;


        typedef cl_int (CL_CALLBACK *clSetKernelArgTy)(cl_kernel,
                                                       cl_uint,
                                                       size_t,
                                                       const void*
                                                      );
        clSetKernelArgTy clSetKernelArgU;

        typedef cl_int(CL_CALLBACK *clEnqueueNDRangeKernelTy)(cl_command_queue,
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

        typedef cl_int (CL_CALLBACK *clGetKernelInfoTy)(cl_kernel,
                                                cl_kernel_info,
                                                size_t,
                                                void *,
                                                size_t *);
        clGetKernelInfoTy clGetKernelInfoU;

        typedef cl_int (CL_CALLBACK *clEnqueueReadBufferTy)(cl_command_queue,
          cl_mem,
          cl_bool,
          size_t,
          size_t,
          void *,
          cl_uint,
          const cl_event *,
          cl_event *);

        clEnqueueReadBufferTy clEnqueueReadBufferU;

        UnderlyingCaller();

        static UnderlyingCaller& Singleton();
};

}

#endif
