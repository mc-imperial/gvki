set(SOURCES InterceptedHostFunctions.cpp UnderlyingCaller.cpp Logger.cpp)

# The LD_PRELOAD library
if (NOT MSYS)
    add_library(GVKI_preload SHARED ${SOURCES})
endif()

# The Macro style library
add_library(GVKI_macro STATIC ${SOURCES})
set_property(TARGET GVKI_macro
             APPEND
             PROPERTY COMPILE_DEFINITIONS "MACRO_LIB"
            )
