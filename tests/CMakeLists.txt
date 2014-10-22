# Macro for creating a tests
macro(GVKI_TEST test)
    get_filename_component(testWithoutExt ${test} NAME_WE)

    # FIXME: We should check that ${test} contains no spaces

    # Preload library. It might not be built on all hosts
    if (TARGET GVKI_preload)
        add_executable(${testWithoutExt}_gvki_preload EXCLUDE_FROM_ALL ${testWithoutExt})
        target_link_libraries(${testWithoutExt}_gvki_preload ${OPENCL_LIBRARIES})
        add_dependencies(${testWithoutExt}_gvki_preload GVKI_preload)
        add_dependencies(check ${testWithoutExt}_gvki_preload)
    endif()

    # Macro library
    add_executable(${testWithoutExt}_gvki_macro EXCLUDE_FROM_ALL ${testWithoutExt})
    target_compile_definitions(${testWithoutExt}_gvki_macro PRIVATE MACRO_LIB)
    target_link_libraries(${testWithoutExt}_gvki_macro GVKI_macro ${OPENCL_LIBRARIES})
    add_dependencies(check ${testWithoutExt}_gvki_macro)
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
