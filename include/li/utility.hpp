#ifndef H_UTILITY
#define H_UTILITY

#include <string>

namespace lisp {

namespace interpreter {

// Error reporting
void throw_error(std::string error_string);

// Throw error to be caught (recoverable)
void assert_throw(const char * loc, std::string msg, bool condition);

}

}

#endif