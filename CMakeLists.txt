project(gvki)
cmake_minimum_required(VERSION 2.8.7)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/findopencl")

###############################################################################
# Test supported compiler flags
###############################################################################
# TODO

###############################################################################
# Find OpenCL headers
###############################################################################
# We don't necessarily need the library
find_package(OpenCL)

# It's possible to build without the library itself
# on some platforms.
if (OPENCL_INCLUDE_DIRS)
    include_directories(${OPENCL_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Can't build without OpenCL header files")
endif()

# The library and the header file were found
if (OPENCL_FOUND)
    list(GET OPENCL_LIBRARIES 0 OPENCL_LIBRARY_ABS_PATH)

    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        # On OSX OPENCL_LIBRARY_ABS_PATH is actually a framework folder
        # so we need to append the actually library file to the path
        if (IS_DIRECTORY ${OPENCL_LIBRARY_ABS_PATH})
            set(OPENCL_LIBRARY_ABS_PATH "${OPENCL_LIBRARY_ABS_PATH}/OpenCL")
        else()
            message(FATAL_ERROR "Expected \"${OPENCL_LIBRARY_ABS_PATH}\" to be a directory on OSX")
        endif()
    endif()

    # Check the library path actually exists
    if(NOT EXISTS "${OPENCL_LIBRARY_ABS_PATH}")
        message(FATAL_ERROR "Expected path to OpenCL library \"${OPENCL_LIBRARY_ABS_PATH}\" does not exist")
    endif()
else()
    set(OPENCL_LIBRARY_ABS_PATH "UNKNOWN")

    # We must have the OpenCL library on OSX
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        message(FATAL_ERROR "OpenCL library is required on OSX")
    endif()

endif()

###############################################################################
# Setup includes
###############################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include)

add_subdirectory(include)
add_subdirectory(lib)

option(ENABLE_TESTING OFF)
if (ENABLE_TESTING)
    # We definitely need the OpenCL library to run the tests
    if (NOT OPENCL_LIBRARIES)
        message(FATAL_ERROR "An OpenCL library is required to build and run tests")
    endif ()

    add_subdirectory(tests)
endif ()
