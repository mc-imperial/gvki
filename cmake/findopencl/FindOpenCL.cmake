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

    # There's no standard place for the header files and libraries to live
    # so we hard code a few guesses. If the header files or libraries aren't
    # found then the user will need to specify their location manually by doing
    # something like
    #
    # cmake -DOPENCL_INCLUDE_DIRS="/some/path/include" -DOPENCL_LIBRARIES="/some/path/OpenCL.a"
    #
    # or use cmake-gui to pick the file/directory.
    find_path(OPENCL_INCLUDE_DIRS
              NAMES CL/cl.h CL/cl.hpp
              # FIXME: Use system introspection to provide HINTS instead
              PATHS "C:\\Program Files (x86)\\AMD APP SDK\\2.9-1\\include"
             )

    if (NOT OPENCL_INCLUDE_DIRS)
        message(WARNING "Could not find OpenCL header file. You will need to specify the location manually")
    endif()
    find_library(OPENCL_LIBRARIES
                 NAMES OpenCL
                 # FIXME: Use system introspection to provide HINTS instead
                 PATHS "C:\\Program Files (x86)\\AMD APP SDK\\2.9-1\\lib\\x86_64"
                 PATHS "C:\\Program Files (x86)\\AMD APP SDK\\2.9-1\\lib\\x86"
                 DOC "Path to OpenCL library"
                )

    if (NOT OPENCL_LIBRARIES)
        message(WARNING "Could not find OpenCL library. You will need to specify the location manually")
    endif()
else ()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Quick sanity check
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # FIXME: This is fragile. Perhaps we shouldn't do the sanity
    #        check at all on OSX?
    set(CL_HEADER_PATH "${OPENCL_INCLUDE_DIRS}/Headers/cl.h")
else()
    set(CL_HEADER_PATH "${OPENCL_INCLUDE_DIRS}/CL/cl.h")
endif()
if (OPENCL_INCLUDE_DIRS)
    if (NOT EXISTS "${CL_HEADER_PATH}")
        message(FATAL_ERROR "Could not find OpenCL header file in ${CL_HEADER_PATH}")
    endif()
endif()

# Handle QUIET and REQUIRED and check the necessary variables were set
find_package_handle_standard_args(OpenCL DEFAULT_MSG OPENCL_LIBRARIES OPENCL_INCLUDE_DIRS)

message(STATUS "OPENCL_INCLUDE_DIRS are ${OPENCL_INCLUDE_DIRS}")
message(STATUS "OPENCL_LIBRARIES are ${OPENCL_LIBRARIES}")
