#ifndef GVKILL_DEBUG_H
#define GVKILL_DEBUG_H
#include <cstdlib>
#include <iostream>
#include "gvki/GlobalLogFile.h"

// FIXME: These need to be made Windows comptabile
#define DEBUG(X) if (getenv("GVKI_DEBUG") != NULL) X
#define DEBUG_MSG(X) DEBUG( std::cerr << "\033[32m***GVKI:" << X  << "***\033[0m" << std::endl); \
                     /* We always want debug messages in the log */ \
                     gvki::GlobalLogFile::singleton() << X << "\n"

// FIXME: This belongs in its own header file
#define ERROR_MSG(X) std::cerr << "\033[31m**GVKI ERROR:" << X << "***\033\[0m" << std::endl; \
                     gvki::GlobalLogFile::singleton() << X << "\n"

#endif
