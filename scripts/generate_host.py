#!/usr/bin/env python

# Code based on the metamorphiccl project (see
# metamorphiccl/python/replayer.py there)

import os
import sys
import json

def get_scalar_host_typename(size_in_bytes):
    if size_in_bytes == 4:
        return "cl_uint"
    if size_in_bytes == 2:
        return "cl_ushort"
    if size_in_bytes == 1:
        return "cl_uchar"
    assert(False)

def emit_opencl_target(outfile):
    outfile.write('''
//---- opencl_target (see https://github.com/hevrard/opencl_target) ---------
#define OPENCL_TARGET_CL_CALL(func, ...) do {                           \\
        cl_int __cl_err = 0;                                            \\
        __cl_err = func(__VA_ARGS__);                                   \\
        if (__cl_err != CL_SUCCESS) {                                   \\
            fprintf(stderr, "%s:%d OPENCL_TARGET PANIC: OpenCL primitive failed: %s\\n", __FILE__, __LINE__, #func ); \\
            abort();                                                    \\
        }                                                               \\
    } while (0)

void opencl_target_init(cl_platform_id *platform_id, cl_device_id *device_id);

inline void opencl_target_init(cl_platform_id *platform_id, cl_device_id *device_id)
{
    cl_platform_id *platforms = NULL;
    cl_device_id *devices = NULL;
    char *device_version = NULL;
    *device_id = NULL;

    char *envname;
    envname = getenv("OPENCL_TARGET_DEVICE");
    if (envname == NULL) {
        envname = (char *)"";   // cast to please c++ compiler
    }

    cl_uint num_platforms = 0;
    OPENCL_TARGET_CL_CALL (clGetPlatformIDs, 0, NULL, &num_platforms);
    if (num_platforms <= 0) {
        goto opencl_target_exit;
    }

    platforms = (cl_platform_id *) malloc(num_platforms * sizeof(cl_platform_id));
    OPENCL_TARGET_CL_CALL (clGetPlatformIDs, num_platforms, platforms, NULL);

    for (cl_uint i = 0; i < num_platforms; i++) {
        *platform_id = platforms[i];

        cl_uint num_devices = 0;
        OPENCL_TARGET_CL_CALL (clGetDeviceIDs, *platform_id, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
        if (num_devices <= 0) {
            continue;
        }

        devices = (cl_device_id *) realloc(devices, num_devices * sizeof(cl_device_id));
        OPENCL_TARGET_CL_CALL (clGetDeviceIDs, *platform_id, CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);

        for (cl_uint j = 0; j < num_devices; j++) {
            *device_id = devices[j];
            size_t device_version_size = 0;
            OPENCL_TARGET_CL_CALL (clGetDeviceInfo, *device_id, CL_DEVICE_VERSION, 0, NULL, &device_version_size);
            if (device_version_size <= 0) {
                continue;
            }
            device_version = (char *) realloc(device_version, device_version_size);
            if (device_version == NULL) {
                goto opencl_target_exit;
            }
            OPENCL_TARGET_CL_CALL (clGetDeviceInfo, *device_id, CL_DEVICE_VERSION, device_version_size, device_version, NULL);
            int name_match = ( strstr(device_version, envname) != NULL );
            free(device_version);
            device_version = NULL;
            if (name_match) {
                goto opencl_target_exit;
            }
        }
    }

    // reaching here means no device was found;
    *device_id = NULL;

opencl_target_exit:

    if (platforms      != NULL) { free(platforms); }
    if (devices        != NULL) { free(devices); }
    if (device_version != NULL) { free(device_version); }
}

#undef OPENCL_TARGET_CL_CALL
//-----------------------------------------------------------------------------
''')

def emit_set_kernel_arg(__indent, __i, __type, __name, outfile):
    outfile.write(__indent + "err = clSetKernelArg(kernel, kernel_arg++, sizeof(" + __type + "), &" + __name + ");\n")
    outfile.write(__indent + "if(cl_error_check(err, \"Error setting kernel argument " + str(__i) + "\"))\n")
    outfile.write(__indent + "  exit(1);\n")

def emit_array_print_loop(__cl_type, flag, outfile):
    outfile.write("  printf(\"[\");\n")
    outfile.write("  for(size_t i = 0; i < (size / sizeof(" + __cl_type + ")); ++i) {\n")
    outfile.write("    if(i > 0) printf(\",\");\n")
    outfile.write("    printf(\"%" + flag + "\", ((" + __cl_type + "*)data)[i]);\n")
    outfile.write("  }\n")
    outfile.write("  printf(\"]\\n\");\n")

def emit_includes(outfile):
    outfile.write("""
#ifdef __APPLE__
  #include <OpenCL/OpenCL.h>
#else
  #include <CL/cl.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

""")

def emit_error_handlers(outfile):
    outfile.write("int cl_error_check(cl_int err, const char *err_string) {\n")
    outfile.write("  if (err == CL_SUCCESS)\n")
    outfile.write("    return 0;\n")
    outfile.write("  printf(\"%s: %d\\n\", err_string, err);\n")
    outfile.write("  return 1;\n")
    outfile.write("}\n")
    outfile.write("\n\n")

    outfile.write("// Called if any error occurs during context creation or at kernel runtime.\n")
    outfile.write("// This can be called many time asynchronously, so it must be thread safe.\n")
    outfile.write("void\n")
    outfile.write("#ifdef _MSC_VER\n")
    outfile.write("__stdcall\n")
    outfile.write("#endif\n")
    outfile.write("error_callback(const char * errinfo, const void * private_info, size_t cb, void * user_data) {\n")
    outfile.write("  printf(\"Error found (callback):\\n%s\\n\", errinfo);\n")
    outfile.write("}\n\n")

def emit_show_array(outfile):
    outfile.write("void show_array(const char * data, size_t size, const char * type) {\n")
    outfile.write("  if(strcmp(type, \"int*\") == 0) {\n")
    emit_array_print_loop("cl_int", "d", outfile)
    outfile.write("  } else if(strcmp(type, \"uint*\") == 0) {\n")
    emit_array_print_loop("cl_uint", "d", outfile)
    outfile.write("  } else if(strcmp(type, \"short*\") == 0) {\n")
    emit_array_print_loop("cl_short", "d", outfile)
    outfile.write("  } else if(strcmp(type, \"ushort*\") == 0) {\n")
    emit_array_print_loop("cl_ushort", "d", outfile)
    outfile.write("  } else if(strcmp(type, \"char*\") == 0) {\n")
    emit_array_print_loop("cl_char", "d", outfile)
    outfile.write("  } else if(strcmp(type, \"uchar*\") == 0) {\n")
    emit_array_print_loop("cl_uchar", "d", outfile)
    outfile.write("  } else if(strcmp(type, \"float*\") == 0) {\n")
    emit_array_print_loop("cl_float", "f", outfile)
    outfile.write("  } else {\n")
    outfile.write("    printf(\"Array of unsupported type %s\\n\", type);\n")
    outfile.write("  }\n")
    outfile.write("}\n\n");

def emit_opencl_target_init(outfile):
    outfile.write('''

    opencl_target_init(&platform_id, &device_id);
''')

def emit_check_device_supports_configuration(kernel_info, outfile):
    outfile.write("\n\n  // Checking device supports given number of dimensions\n")
    outfile.write("  cl_int max_dimensions;\n")
    outfile.write("  err = clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_int), &max_dimensions, NULL);\n")
    outfile.write("  if (cl_error_check(err, \"Error querying CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS\"))\n");
    outfile.write("    exit(1);\n")
    outfile.write("  if(max_dimensions < " + str(len(kernel_info["global_size"])) + ") {\n")
    outfile.write("    printf(\"Kernel uses %d dimensions, exceeds the maximum of %d dimensions for this device\\n\", " + str(len(kernel_info["global_size"])) + ", max_dimensions);\n")
    outfile.write("    exit(1);\n")
    outfile.write("  }")

    outfile.write("\n\n  // Checking that number of work items in each dimension is OK\n")
    outfile.write("  size_t * max_work_items = (size_t*)malloc(sizeof(size_t)*max_dimensions);")
    outfile.write("  err = clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t)*max_dimensions, max_work_items, NULL);\n")
    outfile.write("  if (cl_error_check(err, \"Error querying CL_DEVICE_MAX_WORK_ITEM_SIZES\"))\n");
    outfile.write("    exit(1);\n")
    for i in range(0, len(kernel_info["local_size"])):
        outfile.write("  if(max_work_items[" + str(i) + "] < " + str(kernel_info["local_size"][i]) + ") {\n")
        outfile.write("    printf(\"Local works size in dimension " + str(i) + " is %d, which exceeds maximum of %d for this device\\n\", " + str(kernel_info["local_size"][i]) + ", max_work_items[" + str(i) + "]);\n")
        outfile.write("    exit(1);\n")
        outfile.write("  }\n")
    outfile.write("  free(max_work_items);\n")

    outfile.write("\n\n  // Checking that work group size is not too large\n\n")
    outfile.write("  size_t max_work_group_size;\n")
    outfile.write("  err = clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_work_group_size, NULL);\n")
    outfile.write("  if (cl_error_check(err, \"Error querying CL_DEVICE_MAX_WORK_GROUP_SIZE\"))\n");
    outfile.write("    exit(1);\n")

    work_group_size = 1
    for x in kernel_info["local_size"]:
        work_group_size *= x

    outfile.write("  if(max_work_group_size < " + str(work_group_size) + ") {\n")
    outfile.write("    printf(\"Kernel work group size is %d, which exceeds the maximum work group size of %d for this device\\n\", " + str(work_group_size) + ", max_work_group_size);\n")
    outfile.write("    exit(1);\n")
    outfile.write("  }")

def emit_create_context(outfile):
    outfile.write("\n\n  // Creating a context\n")
    outfile.write("  cl_context_properties properties[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id, 0 };\n")
    outfile.write("  cl_context context = clCreateContext(properties, 1, &device_id, error_callback, NULL, &err);\n")
    outfile.write("  if (cl_error_check(err, \"Error creating context\"))\n")
    outfile.write("    exit(1);\n")

def emit_create_command_queue(outfile):
    outfile.write("\n\n  // Creating a command queue\n")
    outfile.write("  cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &err);\n")
    outfile.write("  if (cl_error_check(err, \"Error creating command queue\"))\n")
    outfile.write("    exit(1);\n")

def emit_create_program(outfile):
    outfile.write("\n\n  // Creating a program\n")
    outfile.write("  char * source_text = NULL;\n")
    outfile.write("  {\n")
    outfile.write("    FILE * source = fopen(kernel_filename, \"rb\");\n")
    outfile.write("    if (source == NULL) {\n")
    outfile.write("      printf(\"Could not open %s.\\n\", kernel_filename);\n")
    outfile.write("      exit(1);\n")
    outfile.write("    }\n")
    outfile.write("    size_t source_size;\n")
    outfile.write("    char temp[1024];\n")
    outfile.write("    while (!feof(source)) fread(temp, 1, 1024, source);\n")
    outfile.write("    source_size = ftell(source);\n")
    outfile.write("    rewind(source);\n")
    outfile.write("    source_text = (char*)calloc(1, source_size + 1);\n")
    outfile.write("    if (source_text == NULL) {\n")
    outfile.write("      printf(\"Failed to calloc %ld bytes.\\n\", source_size);\n")
    outfile.write("      exit(1);\n")
    outfile.write("    }\n")
    outfile.write("    fread(source_text, 1, source_size, source);\n")
    outfile.write("    fclose(source);\n")
    outfile.write("  }\n")
    outfile.write("  const char * const_source = source_text;\n")
    outfile.write("  cl_program program = clCreateProgramWithSource(context, 1, &const_source, NULL, &err);\n")
    outfile.write("  if (cl_error_check(err, \"Error creating program\"))\n")
    outfile.write("    exit(1);\n")

def emit_build_program(outfile):
    outfile.write("\n\n  // Building the program\n")
    outfile.write("  err = clBuildProgram(program, 0, NULL, \"-I . -D__NATIVE_EXECUTION -cl-kernel-arg-info\", NULL, NULL);\n")
    outfile.write("  if (cl_error_check(err, \"Error building program\")) {\n")
    outfile.write("    size_t err_size;\n")
    outfile.write("    err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &err_size);\n")
    outfile.write("    if (cl_error_check(err, \"Error getting build info\"))\n")
    outfile.write("      return 1;\n")
    outfile.write("    char *err_code = (char*)malloc(err_size);\n")
    outfile.write("    if (err_code == NULL) {\n")
    outfile.write("      printf(\"Failed to malloc %ld bytes\\n\", err_size);\n")
    outfile.write("      return 1;\n")
    outfile.write("    }\n")
    outfile.write("    err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, err_size, err_code, &err_size);\n")
    outfile.write("    if (!cl_error_check(err, \"Error getting build info\"))\n")
    outfile.write("      printf(\"%s\", err_code);\n")
    outfile.write("    free(err_code);\n")
    outfile.write("    exit(1);\n")
    outfile.write("  }\n")

def emit_create_kernel(kernel_info, outfile):
    outfile.write("\n\n  // Creating a kernel\n")
    outfile.write("  cl_kernel kernel = clCreateKernel(program, \"" + kernel_info["entry_point"] + "\", &err);\n")
    outfile.write("  if (cl_error_check(err, \"Error creating kernel\"))\n")
    outfile.write("    exit(1);\n")

def emit_setup_regular_kernel_args(kernel_info, outfile):
    outfile.write("\n\n  // Setting up the arguments\n")
    outfile.write("  cl_uint kernel_arg = 0;\n")
    for i in range(0, len(kernel_info["kernel_arguments"])):
        arg = kernel_info["kernel_arguments"][i]
        outfile.write("\n\n  // Setting up argument " + str(i) + "\n")
        if arg["type"] == "array":
            array_name = "array_arg_" + str(i)
            array_type = "cl_mem"
            # Hugues: in json, arg["flags"] is a list, but here the code expect a single string
            outfile.write("  " + array_type + " " + array_name + " = clCreateBuffer(context, " + " ".join(arg["flags"]) + ", " + str(arg["size"]) + ", NULL, &err);\n")
            outfile.write("  if (cl_error_check(err, \"Error creating buffer for kernel argument " + str(i) + "\"))\n")
            outfile.write("    exit(1);\n")
            outfile.write("  char * array_data_" + str(i) + " = (char*)malloc(sizeof(char)*" + str(arg["size"]) + ");\n")
            outfile.write("  if(!array_data_" + str(i) + ") {\n")
            outfile.write("    printf(\"Error allocating memory on host for argument " + str(i) + "\");\n")
            outfile.write("    exit(1);\n")
            outfile.write("  }\n")
            if ("CL_MEM_READ_ONLY" in arg["flags"]) or ("CL_MEM_READ_WRITE" in arg["flags"]):
                outfile.write("  FILE *array_file_ptr_" + str(i) + " = fopen(\"" + arg["data"] + "\", \"rb\");\n")
                outfile.write("  if(!array_file_ptr_" + str(i) + ") {\n")
                outfile.write("    printf(\"Error opening data file for argument " + str(i) + "\");\n")
                outfile.write("    exit(1);\n")
                outfile.write("  }\n")
                outfile.write("  if(fread(array_data_" + str(i) + ", sizeof(char), " + str(arg["size"]) + ", array_file_ptr_" + str(i) + ") != " + str(arg["size"]) + ") {\n")
                outfile.write("    printf(\"Error reading data file for argument " + str(i) + "\");\n")
                outfile.write("    exit(1);\n")
                outfile.write("  }\n")
            else:
                assert arg["flags"] == [ "CL_MEM_WRITE_ONLY" ]
                outfile.write("  memset(array_data_" + str(i) + ", 0, " + str(arg["size"]) + ");\n")
            outfile.write("  err = clEnqueueWriteBuffer(command_queue, " + array_name + ", CL_TRUE, 0, " + str(arg["size"]) + ", array_data_" + str(i) + ", 0, NULL, NULL);\n")
            outfile.write("  if (cl_error_check(err, \"Error copying to device for argument " + str(i) + "\"))\n")
            outfile.write("    exit(1);\n")
            emit_set_kernel_arg("  ", i, array_type, array_name, outfile)
        elif arg["type"] == "scalar":
            assert(arg["value"].startswith("0x"))
            size_in_bytes = (len(arg["value"]) - 2) / 2
            host_type = get_scalar_host_typename(size_in_bytes)
            host_name = "scalar_arg_" + str(i)
            outfile.write("  " + host_type + " " + host_name + " = " + arg["value"] + ";\n")
            emit_set_kernel_arg("  ", i, host_type, host_name, outfile)
        else:
            sys.stderr.write("Only array and scalar arguments are supported")
            exit(1)

def emit_launch_kernel(kernel_info, outfile):
    outfile.write("\n\n  // Launching the kernel\n")
    assert(len(kernel_info["global_size"]) == len(kernel_info["local_size"]))
    outfile.write("  size_t global_size[" + str(len(kernel_info["global_size"])) + "] = { " + ", ".join([str(s) for s in kernel_info["global_size"]]) + " };\n")
    outfile.write("  size_t local_size[" + str(len(kernel_info["local_size"])) + "] = { " + ", ".join([str(s) for s in kernel_info["local_size"]]) + " };\n")
    outfile.write("  err = clEnqueueNDRangeKernel(command_queue, kernel, " + str(len(kernel_info["global_size"])) + ", NULL, global_size, local_size, 0, NULL, NULL);\n")
    outfile.write("  if(cl_error_check(err, \"Error enqueueing kernel\"))\n")
    outfile.write("    exit(1);\n")
    outfile.write("  err = clFinish(command_queue);\n")
    outfile.write("  if(cl_error_check(err, \"Error sending finish command\"))\n")
    outfile.write("    exit(1);\n")

def emit_copy_back_results(kernel_info, outfile):
    outfile.write("\n\n  // Copying back results\n")
    for i in range(0, len(kernel_info["kernel_arguments"])):
        arg = kernel_info["kernel_arguments"][i]
        if arg["type"] == "array":
            array_name = "array_arg_" + str(i)
            outfile.write("  err = clEnqueueReadBuffer(command_queue, " + array_name + ", CL_TRUE, 0, " + str(arg["size"]) + ", array_data_" + str(i) + ", 0, NULL, NULL);\n")
            outfile.write("  if (cl_error_check(err, \"Error copying results from device for argument " + str(i) + "\"))\n")
            outfile.write("    exit(1);\n")

def emit_write_results(kernel_info, outfile):
    outfile.write("\n\n  // Writing out results\n")
    for i in range(0, len(kernel_info["kernel_arguments"])):
        arg = kernel_info["kernel_arguments"][i]
        if arg["type"] == "array":
            if (arg["flags"] == "CL_MEM_READ_ONLY"):
                continue # We do not want to dump a read-only array in the output
            outfile.write("\n\n  // Getting info about the element type of array argument " + str(i) + "\n")
            outfile.write("  {\n")
            outfile.write("    char type_name[256];\n")
            outfile.write("    size_t type_name_size;\n")
            outfile.write("    err = clGetKernelArgInfo(kernel, " + str(i) + ", CL_KERNEL_ARG_TYPE_NAME, 256, type_name, &type_name_size);\n")
            outfile.write("      if(cl_error_check(err, \"Error querying type of argument " + str(i) + "\"))\n")
            outfile.write("        exit(1);\n")
            outfile.write("    show_array(array_data_" + str(i) + ", " + str(arg["size"]) + ", type_name);\n")
            outfile.write("  }\n")

def emit_cleanup(kernel_info, outfile):
    outfile.write("\n\n  // Freeing host buffers\n")
    for i in range(0, len(kernel_info["kernel_arguments"])):
        arg = kernel_info["kernel_arguments"][i]
        if arg["type"] == "array":
            outfile.write("  free(array_data_" + str(i) + ");\n")
    outfile.write("\n\n  // Freeing allocated memory\n")
    outfile.write("  free(source_text);\n")

def emit_host_application(kernel_info, outfile):
    if kernel_info["language"] != "OpenCL":
        sys.stderr.write("Only OpenCL kernels are supported")
        exit(1)

    assert len(kernel_info["global_size"]) == len(kernel_info["local_size"])

    emit_includes(outfile)
    emit_opencl_target(outfile)
    emit_error_handlers(outfile)
    emit_show_array(outfile)

    outfile.write("int main(int argc, char * * argv) {\n")
    outfile.write("  char *kernel_filename=\"" + kernel_info['kernel_file'] + "\";\n")
    outfile.write("  cl_int err;\n")
    outfile.write("  cl_platform_id platform_id = NULL;\n")
    outfile.write("  cl_device_id device_id = NULL;\n")

    emit_opencl_target_init(outfile)
    # emit_get_platform(outfile)
    # emit_get_device(outfile)
    emit_check_device_supports_configuration(kernel_info, outfile)
    emit_create_context(outfile)
    emit_create_command_queue(outfile)
    emit_create_program(outfile)
    emit_build_program(outfile)
    emit_create_kernel(kernel_info, outfile)
    emit_setup_regular_kernel_args(kernel_info, outfile)
    emit_launch_kernel(kernel_info, outfile)
    emit_copy_back_results(kernel_info, outfile)
    emit_write_results(kernel_info, outfile)
    emit_cleanup(kernel_info, outfile)

    outfile.write("}\n")

    outfile.close()

def usage():
    print("Usage: " + sys.argv[0] + " log.json [ <kernel-info-index> ]")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Need json log as argument")
        usage()
        exit(1)

    gvkilog = sys.argv[1]
    kindex = ""
    if len(sys.argv) == 3:
        kindex = sys.argv[2]

    kernels = {}
    with open(gvkilog, "r") as f:
        kernels = json.load(f)

    if kindex != "":
        kernels = [ kernels[int(kindex)] ]

    for i, k in enumerate(kernels):
        outfile = "minihost_" + str(i) + ".c"
        with open(outfile, "w") as f:
            emit_host_application(k, f)
