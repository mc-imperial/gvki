#ifndef GVKILL_DEBUG_H
#define GVKILL_DEBUG_H
#include <cstdlib>
#include <iostream>

// FIXME: These need to be made Windows comptabile
#define DEBUG(X) if (getenv("GVKILL_DEBUG") != NULL) X
#define DEBUG_MSG(X) DEBUG( std::cerr << "\033[32m***GVKILL:" << X  << "***\033[0m" << std::endl)

// FIXME: This belongs in its own header file
#define ERROR_MSG(X) std::cerr << "\033[31m**GVKILL ERROR:" << X << "***\033\[0m" << std::endl

#endif
