set(SOURCES InterceptedHostFunctions.cpp UnderlyingCaller.cpp Logger.cpp)

# The LD_PRELOAD library
if (NOT WIN32)
    add_library(GVKI_preload SHARED ${SOURCES})

    if (APPLE)
        # Apparently this is necessary on OSX
        set_property(TARGET GVKI_preload APPEND PROPERTY LINK_FLAGS "-flat_namespace")
    endif()
endif()

# The Macro style library
add_library(GVKI_macro STATIC ${SOURCES})
set_property(TARGET GVKI_macro
             APPEND
             PROPERTY COMPILE_DEFINITIONS "MACRO_LIB"
            )
