project(gvki)
cmake_minimum_required(VERSION 2.8.12)

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
if (OPENCL_FOUND)
    include_directories(${OPENCL_INCLUDE_DIRS})
endif ()

###############################################################################
# Setup includes
###############################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(lib)

option(ENABLE_TESTS OFF)
if (ENABLE_TESTS)
    # We definitely need the OpenCL library to run the tests
    if (NOT OPENCL_LIBRARIES)
        message(FATAL_ERROR "An OpenCL library is required to build and run tests")
    endif ()

    add_subdirectory(tests)
endif ()
