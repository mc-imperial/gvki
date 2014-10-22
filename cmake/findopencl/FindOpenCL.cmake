# Finds an installed OpenCL implementation
#
# Sets the folling
# OPENCL_FOUND - True if OpenCL implementation was found
# OPENCL_LIBRARIES - List of libraries
# OPENCL_INCLUDE_DIRS - List of include directories

include(FindPackageHandleStandardArgs)

if (UNIX OR APPLE)
    find_library(OPENCL_LIBRARIES
                 NAMES OpenCL
                 DOC "Path to OpenCL library"
                 # The user might have the library in their LD_LIBRARY_PATH
                 PATHS ENV LD_LIBRARY_PATH
                )

    find_path(OPENCL_INCLUDE_DIRS
              NAMES CL/cl.h OpenCL/cl.h CL/cl.hpp OpenCL/cl.hpp
             )
elseif (WIN32)
    message(FATAL_ERROR "Windows support not implemented")
else ()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Handle QUIET and REQUIRED and check the necessary variables were set
find_package_handle_standard_args(OpenCL DEFAULT_MSG OPENCL_LIBRARIES OPENCL_INCLUDE_DIRS)

set(OPENCL_FOUND TRUE)
