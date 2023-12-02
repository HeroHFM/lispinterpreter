#ifndef H_BUILTINS
#define H_BUILTINS

#include "li/ast.hpp"

#include <functional>

namespace lisp {

namespace interpreter {

struct Builtins {
	using arg_list = ASTNode::node_list;
	const std::unordered_map<std::string, BuiltinNode::builtin_fxn> functions = {
		// Integers
		{"*", [](arg_list & args){
			enforce_all_numeric("*", args);
			return std::make_unique<IntNode>(
				std::accumulate(args.cbegin(), args.cend(), 1, [](int a, const auto & b){
					return a * b->get_numeric();
				})
			);
		}},
		{"+", [](arg_list & args){
			enforce_all_numeric("+", args);
			return std::make_unique<IntNode>(
				std::accumulate(args.cbegin(), args.cend(), 0, [](int a, const auto & b){
					return a + b->get_numeric();
				})
			);
		}},
		{"-", [](arg_list & args){
			enforce_min_arg_count("-", args, 1);
			enforce_all_numeric("-", args);
			return std::make_unique<IntNode>(
				std::accumulate(std::next(args.cbegin()), args.cend(), args.front()->get_numeric(), [](int a, const auto & b){
					return a - b->get_numeric();
				})
			);
		}},
		{"/", [](arg_list & args){
			enforce_min_arg_count("/", args, 1);
			enforce_all_numeric("/", args);
			return std::make_unique<IntNode>(
				std::accumulate(std::next(args.cbegin()), args.cend(), args.front()->get_numeric(), [](int a, const auto & b){
					if ( b->get_numeric() == 0) { throw_error("runtime: division by zero"); } 
					return a / b->get_numeric();
				})
			);
		}},

		{"max", [](arg_list & args){
			enforce_all_numeric("max", args);
			enforce_min_arg_count("max", args, 1);
			return std::make_unique<IntNode>(
				(*std::max_element(args.cbegin(), args.cend(), [](const auto & a, const auto & b){
						return a->get_numeric() < b->get_numeric();
				}))->get_numeric()
			);
		}},
		{"min", [](arg_list & args){
			enforce_all_numeric("min", args);
			enforce_min_arg_count("min", args, 1);
			return std::make_unique<IntNode>(
				(*std::min_element(args.cbegin(), args.cend(), [](const auto & a, const auto & b){
						return a->get_numeric() < b->get_numeric();
				}))->get_numeric()
			);
		}},

		{"=", [](arg_list & args){
			enforce_all_numeric("=", args);
			return std::make_unique<BoolNode>(
				(std::adjacent_find(args.cbegin(), args.cend(), [](const auto & a, const auto & b){
						return a->get_numeric() != b->get_numeric();
				})) == args.cend()
			);
		}},
		{"<", [](arg_list & args){
			enforce_all_numeric("<", args);
			return std::make_unique<BoolNode>(
				(std::adjacent_find(args.cbegin(), args.cend(), [](const auto & a, const auto & b){
						return a->get_numeric() >= b->get_numeric();
				})) == args.cend()
			);
		}},
		{">", [](arg_list & args){
			enforce_all_numeric(">", args);
			return std::make_unique<BoolNode>(
				(std::adjacent_find(args.cbegin(), args.cend(), [](const auto & a, const auto & b){
						return a->get_numeric() <= b->get_numeric();
				})) == args.cend()
			);
		}},
		{"<=", [](arg_list & args){
			enforce_all_numeric("<=", args);
			return std::make_unique<BoolNode>(
				(std::adjacent_find(args.cbegin(), args.cend(), [](const auto & a, const auto & b){
						return a->get_numeric() > b->get_numeric();
				})) == args.cend()
			);
		}},
		{">=", [](arg_list & args){
			enforce_all_numeric(">=", args);
			return std::make_unique<BoolNode>(
				(std::adjacent_find(args.cbegin(), args.cend(), [](const auto & a, const auto & b){
						return a->get_numeric() < b->get_numeric();
				})) == args.cend()
			);
		}},

		{"abs", [](arg_list & args){
			enforce_arg_exact_count("abs", args, 1);
			enforce_all_numeric("abs", args);
			return std::make_unique<IntNode>(
				std::abs(args.front()->get_numeric())
			);
		}},
		{"expt", [](arg_list & args){
			enforce_arg_exact_count("expt", args, 2);
			enforce_all_numeric("expt", args);
			return std::make_unique<IntNode>(
				std::pow(args.front()->get_numeric(),
					     args.back()->get_numeric())
			);
		}},
		{"modulo", [](arg_list & args){
			enforce_arg_exact_count("modulo", args, 2);
			enforce_all_numeric("modulo", args);
			if (args.back()->get_numeric() == 0) { throw_error("runtime: division by zero"); }
			return std::make_unique<IntNode>(
				args.front()->get_numeric() % args.back()->get_numeric()
			);
		}},
		{"zero?", [](arg_list & args){
			enforce_arg_exact_count("zero?", args, 1);
			enforce_all_numeric("zero?", args);
			return std::make_unique<BoolNode>(
				args.front()->get_numeric() == 0
			);
		}},
		// Pairs
		{"car", [](arg_list & args){
			enforce_arg_exact_count("car", args, 1);
			return args.front()->get(0);
		}},
		{"cdr", [](arg_list & args){
			enforce_arg_exact_count("cdr", args, 1);
			return args.front()->get(1);
		}},
		// Lists
		{"length", [](arg_list & args){
			enforce_arg_exact_count("length", args, 1);
			enforce_all_list("length", args);

			std::function<std::size_t(const node_ptr, std::size_t)> get_length = [&get_length](const node_ptr node, std::size_t count){
				if (node->is_unit()) { return count; }
				return get_length(node->get(1), count + 1);
			};

			return std::make_unique<IntNode>(get_length(args.front(), 0));
		}},
		{"append", [](arg_list & args){
			enforce_arg_exact_count("append", args, 2);
			enforce_all_list("append", args);

			std::function<node_ptr(node_ptr, node_ptr)>
			concat = [&concat](node_ptr l, node_ptr r){
				if (l->is_unit()) { return r; }
				return std::dynamic_pointer_cast<ASTNode>(std::make_shared<PairNode>(l->get(0), concat(l->get(1), r)));
			};

			return concat(args.front(), args.back());
		}},
		// Other
		{"display", [](arg_list & args){
			enforce_arg_exact_count("display", args, 1);
			std::cout << *args.front() << std::flush;
			return std::make_unique<NullNode>();
		}},
		{"newline", [](arg_list & args){
			enforce_arg_exact_count("newline", args, 0);
			std::cout << std::endl;
			return std::make_unique<NullNode>();
		}},
		{"not", [](arg_list & args){
			enforce_arg_exact_count("newline", args, 1);
			return std::make_unique<BoolNode>(!args.front()->get_boolean());
		}},
		// Types
		{"boolean?", [](arg_list & args){
			enforce_arg_exact_count("boolean?", args, 1);
			return std::make_unique<BoolNode>(args.front()->is_boolean());
		}},
		{"integer?", [](arg_list & args){
			enforce_arg_exact_count("integer?", args, 1);
			return std::make_unique<BoolNode>(args.front()->is_numeric());
		}},
		{"pair?", [](arg_list & args){
			enforce_arg_exact_count("pair?", args, 1);
			return std::make_unique<BoolNode>(args.front()->is_pair());
		}},
		{"list?", [](arg_list & args){
			enforce_arg_exact_count("list?", args, 1);
			return std::make_unique<BoolNode>(is_list(args.front()));
		}},
		{"procedure?", [](arg_list & args){
			enforce_arg_exact_count("procedure?", args, 1);
			return std::make_unique<BoolNode>(args.front()->is_callable());
		}},
		{"null?", [](arg_list & args){
			enforce_arg_exact_count("null?", args, 1);
			return std::make_unique<BoolNode>(args.front()->is_unit());
		}},
	};
};

}

}

#endif