# Finds an installed OpenCL implementation
#
# Sets the folling
# OPENCL_FOUND - True if OpenCL implementation was found
# OPENCL_LIBRARIES - List of libraries
# OPENCL_INCLUDE_DIRS - List of include directories

include(FindPackageHandleStandardArgs)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    ###########################################################################
    # Linux
    ###########################################################################
    find_library(OPENCL_LIBRARIES
                 NAMES OpenCL
                 DOC "Path to OpenCL library"
                 # The user might have the library in their LD_LIBRARY_PATH
                 PATHS ENV LD_LIBRARY_PATH
                )

    find_path(OPENCL_INCLUDE_DIRS
              NAMES CL/cl.h CL/cl.hpp
             )

    if ( CMAKE_SYSTEM_PROCESSOR MATCHES "^arm" )
        # For ARM's Mali libOpenCL.so does not have the
        # OpenCL symbols. These are actually in libmali
        find_library(MALI_LIBRARY mali)
        if( MALI_LIBRARY )
            # Found Mali library so append to OpenCL library list
            list(APPEND OPENCL_LIBRARIES ${MALI_LIBRARY})
            message(STATUS "Found ARM Mali library ${MALI_LIBRARY}")
        else()
            message(WARNING "ARM target detected but Mali library was not found.")
        endif()
    endif()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    ###########################################################################
    # OSX (Darwin)
    ###########################################################################
    find_library(OPENCL_LIBRARIES
                 NAMES OpenCL
                 DOC "Path to OpenCL library"
                )

    find_path(OPENCL_INCLUDE_DIRS
              NAMES OpenCL/cl.h OpenCL/cl.hpp
             )

elseif (WIN32)
    ###########################################################################
    # Windows
    ###########################################################################
    message(FATAL_ERROR "Windows support not implemented")
else ()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Handle QUIET and REQUIRED and check the necessary variables were set
find_package_handle_standard_args(OpenCL DEFAULT_MSG OPENCL_LIBRARIES OPENCL_INCLUDE_DIRS)

message(STATUS "OPENCL_INCLUDE_DIRS are ${OPENCL_INCLUDE_DIRS}")
message(STATUS "OPENCL_LIBRARIES are ${OPENCL_LIBRARIES}")
