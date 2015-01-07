#ifndef GVKI_PATH_SEPERATOR
#define GVKI_PATH_SEPERATOR

// Provide a macro with the path seperator
// for the platform we are building for
// FIXME: Maybe we should get the host from CMake instead
#if _WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

#endif
