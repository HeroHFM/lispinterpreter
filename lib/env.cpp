#include "li/env.hpp"
#include "li/ast.hpp"

#include <format>
#include <algorithm>
#include<forward_list>
#include <memory>

namespace lisp {

namespace interpreter {

void enforce_arg_exact_count(const char * fname, node_list & args, std::size_t count)
{
	assert_throw(
		fname,
		std::format("expected exactly {} args, got {}", count, args.size()),
		args.size() == count
	);
}

void enforce_min_arg_count(const char * fname, node_list & args, std::size_t count)
{
	assert_throw(
		fname,
		std::format("expected at least {} args, got {}", count, args.size()),
		args.size() >= count
	);
}

void enforce_all_numeric(const char * fname, node_list & args)
{
	assert_throw(
		fname,
		std::format("all arguments must be numeric"),
		std::all_of(args.cbegin(), args.cend(), [](const auto & node){
			return node->is_numeric();
		})
	);
}

void enforce_all_boolean(const char * fname, node_list & args)
{
	assert_throw(
		fname,
		std::format("all arguments must be boolean"),
		std::all_of(args.cbegin(), args.cend(), [](const auto & node){
			return node->is_boolean();
		})
	);
}

void enforce_all_list(const char * fname, node_list & args)
{
	assert_throw(
		fname,
		std::format("argument(s) must be of type list"),
		std::all_of(args.cbegin(), args.cend(), [](const auto & node){
			return is_list(node);
		})
	);
}

Env::Env(std::unordered_map<std::string, node_ptr> * tl, const std::unordered_map<std::string, builtin_fxn> * bt) : toplvl_(tl), builtins_(bt) { }
void Env::insert(const std::string name, node_ptr value, bool top) {
	if (top) { toplvl_->insert_or_assign(name, value); }
	else     { bindings_.insert_or_assign(name, value);}
}

node_ptr Env::find(const std::string name)
{
	// Check immediate environment (`let`s)
	if (bindings_.count(name)) { return bindings_.find(name)->second; }

	// Check top level (`begin`s)
	if (toplvl_->count(name)) { return toplvl_->find(name)->second; }

	// Check builtins
	if (builtins_->count(name)) {
		return std::make_unique<BuiltinNode>(name, builtins_->find(name)->second);
	}

	// Report not found
	throw_error("unbound variable: " + name);
	return nullptr;
}

}

}