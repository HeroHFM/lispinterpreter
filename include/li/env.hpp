#ifndef H_ENV
#define H_ENV

#include "li/utility.hpp"

#include <forward_list>
#include <utility>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <iterator>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <list>

namespace lisp {

namespace interpreter {

class ASTNode;
using node_ptr = std::shared_ptr<ASTNode>;
using node_list = std::list<node_ptr>;
class BuiltinNode;
using builtin_fxn = std::function<node_ptr(node_list &)>;

// Enforcing constrains for builtin functions
void enforce_arg_exact_count(const char * fname, node_list & args, std::size_t count);
void enforce_min_arg_count(const char * fname, node_list & args, std::size_t count);
void enforce_all_numeric(const char * fname, node_list & args);
void enforce_all_boolean(const char * fname, node_list & args);
void enforce_all_list(const char * fname, node_list & args);

class Env {
public:
	using kv_pair = std::pair<const std::string, node_ptr>;

	Env() = default;
	Env(std::unordered_map<std::string, node_ptr> * tl, const std::unordered_map<std::string, builtin_fxn> * bt);
	void insert(std::string name, node_ptr value, bool top);
	node_ptr find(const std::string name);

private:
	// Constructed Environment
	std::unordered_map<std::string, node_ptr> bindings_;
	// Top-Level
	std::unordered_map<std::string, node_ptr> * toplvl_;
	// Built-in functions
	const std::unordered_map<std::string, builtin_fxn> * builtins_;
};

}

}

#endif