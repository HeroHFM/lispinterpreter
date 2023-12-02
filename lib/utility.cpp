#include "li/utility.hpp"

#include <cstdlib>

namespace lisp {

namespace interpreter {

void throw_error(std::string error_string) { throw error_string; }

void assert_throw(const char * loc, std::string msg, bool condition) {
	if (!condition) { throw std::string("procedure `") + loc + "`: " + msg; }
}

}

}
