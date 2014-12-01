# Macro for creating a tests
# The arguments are
# <test> [ <kernel_file> [ <kernel_file> [...] ] ]
#
# <test> - The name of the host code source file to compile and run
# <kernel_file> - The name of a kernel file to copy over to the build directory.
#                 Multiple files can be specified here if necessary
#
# Note this macro should **ONLY BE USED ONCE** inside a directory because
# each test should have its own directory
macro(GVKI_TEST test)
    get_filename_component(testWithoutExt ${test} NAME_WE)

    # FIXME: We should check that ${test} contains no spaces

    # Preload library. It might not be built on all hosts
    if (TARGET GVKI_preload)
        add_executable(${testWithoutExt}_gvki_preload EXCLUDE_FROM_ALL ${testWithoutExt})
        target_link_libraries(${testWithoutExt}_gvki_preload ${OPENCL_LIBRARIES})
        add_dependencies(${testWithoutExt}_gvki_preload GVKI_preload)
        add_dependencies(check ${testWithoutExt}_gvki_preload)

        set(OutputDir ${CMAKE_CURRENT_BINARY_DIR})
        set_target_properties(${testWithoutExt}_gvki_preload
                              PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                         ${OutputDir}
                             )
        # Copy over kernels
        foreach (kernel ${ARGN})
            file(COPY ${kernel} DESTINATION ${OutputDir})
        endforeach()

        # Create logging directory
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gvki_preload.log.d")
    endif()

    # Macro library
    add_executable(${testWithoutExt}_gvki_macro EXCLUDE_FROM_ALL ${testWithoutExt})

    # Can't use target_compile_definitions() here because we need support CMake 2.8.7
    set_target_properties(${testWithoutExt}_gvki_macro PROPERTIES COMPILE_DEFINITIONS MACRO_LIB)
    target_link_libraries(${testWithoutExt}_gvki_macro GVKI_macro ${OPENCL_LIBRARIES})
    add_dependencies(check ${testWithoutExt}_gvki_macro)

    # Ensure each test goes in its own directory to simplify testing
    set(OutputDir ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties(${testWithoutExt}_gvki_macro
                          PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                          ${OutputDir}
                         )
    # Copy over kernels
    foreach (kernel ${ARGN})
        file(COPY ${kernel} DESTINATION ${OutputDir})
    endforeach()

    # Create logging directory
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gvki_macro.log.d")
endmacro()

find_package(PythonInterp REQUIRED)
# Custom target to run tests

if (TARGET GVKI_preload)
    # FIXME: Make CMake 3.0 doesn't want to let us read target LOCATION
    # fix this some how!
    get_target_property(GVKI_preload_path GVKI_preload LOCATION)
else()
    set(GVKI_preload_path "none")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.cfg.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.cfg
               @ONLY
              )

if (NOT ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}"))
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/runtests.py
                   ${CMAKE_CURRENT_BINARY_DIR}/runtests.py
                   COPYONLY
                  )
endif()

add_custom_target(check
                  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/runtests.py
                  ${CMAKE_CURRENT_BINARY_DIR}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                  COMMENT "Running tests"
                 )

# Add test directories
add_subdirectory(HelloWorld)
add_subdirectory(HelloWorldGlobalOffset)
add_subdirectory(HelloWorldMultipleInvocations)
add_subdirectory(SimplePrefixSum)
