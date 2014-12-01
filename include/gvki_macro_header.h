#ifndef CL_HOOKS_H
#define CL_HOOKS_H

/* This header file is for use with the lib_interceptor macro library.
 * Include this in your host code just after #include <CL/cl.h> and your
 * calls will be intercepted.
 */

#ifdef __cplusplus
extern "C" {
#endif

extern cl_mem
clCreateBuffer_hook(cl_context   /* context */,
                    cl_mem_flags /* flags */,
                    size_t       /* size */,
                    void *       /* host_ptr */,
                    cl_int *     /* errcode_ret */);


extern cl_mem
clCreateSubBuffer_hook(cl_mem                   /* buffer */,
                       cl_mem_flags             /* flags */,
                       cl_buffer_create_type    /* buffer_create_type */,
                       const void *             /* buffer_create_info */,
                       cl_int *                 /* errcode_ret */);


extern cl_mem
clCreateImage2D_hook(cl_context              /* context */,
                     cl_mem_flags            /* flags */,
                     const cl_image_format * /* image_format */,
                     size_t                  /* image_width */,
                     size_t                  /* image_height */,
                     size_t                  /* image_row_pitch */,
                     void *                  /* host_ptr */,
                     cl_int *                /* errcode_ret */);


extern cl_mem
clCreateImage3D_hook(cl_context              /* context */,
                     cl_mem_flags            /* flags */,
                     const cl_image_format * /* image_format */,
                     size_t                  /* image_width */,
                     size_t                  /* image_height */,
                     size_t                  /* image_depth */,
                     size_t                  /* image_row_pitch */,
                     size_t                  /* image_slice_pitch */,
                     void *                  /* host_ptr */,
                     cl_int *                /* errcode_ret */);


extern cl_sampler
clCreateSampler_hook(cl_context          /* context */,
                     cl_bool             /* normalized_coords */,
                     cl_addressing_mode  /* addressing_mode */,
                     cl_filter_mode      /* filter_mode */,
                     cl_int *            /* errcode_ret */);


extern cl_program
clCreateProgramWithSource_hook(cl_context        /* context */,
                               cl_uint           /* count */,
                               const char **     /* strings */,
                               const size_t *    /* lengths */,
                               cl_int *          /* errcode_ret */);


extern cl_int
clBuildProgram_hook(cl_program           /* program */,
                    cl_uint              /* num_devices */,
                    const cl_device_id * /* device_list */,
                    const char *         /* options */,
                    void (CL_CALLBACK *  /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
                    void *               /* user_data */);


extern cl_kernel
clCreateKernel_hook(cl_program      /* program */,
                    const char *    /* kernel_name */,
                    cl_int *        /* errcode_ret */);

extern cl_int
clCreateKernelsInProgram_hook(cl_program     /* program */,
                              cl_uint        /* num_kernels */,
                              cl_kernel *    /* kernels */,
                              cl_uint *      /* num_kernels_ret */);


extern cl_int
clSetKernelArg_hook(cl_kernel    /* kernel */,
                    cl_uint      /* arg_index */,
                    size_t       /* arg_size */,
                    const void * /* arg_value */);


extern cl_int
clEnqueueNDRangeKernel_hook(cl_command_queue /* command_queue */,
                            cl_kernel        /* kernel */,
                            cl_uint          /* work_dim */,
                            const size_t *   /* global_work_offset */,
                            const size_t *   /* global_work_size */,
                            const size_t *   /* local_work_size */,
                            cl_uint          /* num_events_in_wait_list */,
                            const cl_event * /* event_wait_list */,
                            cl_event *       /* event */);


/* Use macros to rewrite host code to use our hooks.
 *
 * */
#define clCreateBuffer clCreateBuffer_hook
#define clCreateSubBuffer clCreateSubBuffer_hook
#define clCreateImage2D clCreateImage2D_hook
#define clCreateImage3D clCreateImage3D_hook
#define clCreateSampler clCreateSampler_hook
#define clCreateProgramWithSource clCreateProgramWithSource_hook
#define clBuildProgram clBuildProgram_hook
#define clCreateKernel clCreateKernel_hook
#define clCreateKernelsInProgram clCreateKernelsInProgram_hook
#define clSetKernelArg clSetKernelArg_hook
#define clEnqueueNDRangeKernel clEnqueueNDRangeKernel_hook

#ifdef __cplusplus
}
#endif

#endif
