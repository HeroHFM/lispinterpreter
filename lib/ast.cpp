#include "li/ast.hpp"
#include "li/env.hpp"

#include <string>
#include <memory>
#include <algorithm>
#include <cassert>
#include <format>
#include <sstream>
#include <iterator>

namespace lisp {

namespace interpreter {

using node_ptr = typename ASTNode::node_ptr;
using node_list = typename ASTNode::node_list; 

std::ostream& operator<<(std::ostream & os, const ASTNode & node)
{
    os << node.to_string();
    return os;
}

int ASTNode::get_numeric() const
{
	throw_error("non-numeric type cannot be interpreted as an integer");
	return 0;
}

bool ASTNode::get_boolean() const
{
	return true; // Everything but #f is #t for conditionals
}

node_ptr ASTNode::call(node_list &)
{
	throw_error("non-callable type cannot be called");
	return nullptr;
}

node_ptr ASTNode::get(std::size_t) const
{
	throw_error("cannot get element of non-pair type");
	return nullptr;
}

std::string ASTNode::get_identifier() const
{
	throw_error("cannot get identifier of non-variable type");
	return "";
}

// Literals

IntNode::IntNode(int val) : value_(val) { }
int IntNode::get_numeric() const { return value_; }
node_ptr IntNode::eval(Env&) { return shared_from_this(); }
std::string IntNode::to_string() const { return std::to_string(value_); }

BoolNode::BoolNode(bool val) : value_(val) { }
bool BoolNode::get_boolean() const { return value_; }
node_ptr BoolNode::eval(Env&) { return shared_from_this(); }
std::string BoolNode::to_string() const { return std::string(value_ ? "#t" : "#f"); }

node_ptr UnitNode::eval(Env&) { return shared_from_this(); }
std::string UnitNode::to_string() const { return std::string("()"); }

NullNode::NullNode(std::string msg) : msg_(msg) {}
node_ptr NullNode::eval(Env & /* env */) { throw "runtime: cannot evaluate empty return type"; }
std::string NullNode::to_string() const { return msg_; }

// Sequences

SeqNode::SeqNode(node_list && seq) : sequence_(std::move(seq)) { }
node_ptr SeqNode::eval(Env & env) {
	if (sequence_.size() == 0) {
		return std::make_unique<NullNode>();
	}

	auto it = sequence_.cbegin();
	auto end = std::prev(sequence_.cend());
	for (; it != end; ++it) {
		(*it)->eval(env);
	}
	return (*it)->eval(env);
}
std::string SeqNode::to_string() const
{
	std::string out = "#<Seq>[ ";
	bool first = true;
	for (const auto & child : sequence_) {
		if (!first) { out += ", "; }
		out += child->to_string(); 
		first = false;
	}
	return out + " ]";
}

// Bindings

VarNode::VarNode(std::string id) : name_(id) { }
node_ptr VarNode::eval(Env & env ) { return env.find(name_)->eval(env); }
std::string VarNode::get_identifier() const { return name_; }
std::string VarNode::to_string() const { return "#<Var> " + name_; }

BindNode::BindNode(std::string name, node_ptr value) : name_(name), value_(value) { }
node_ptr BindNode::eval(Env & env ) {
	env.insert(name_, value_->eval(env), true);
	return std::make_unique<NullNode>(name_);
}
std::string BindNode::to_string() const { return "#<Bind> (" + name_ + ", " + value_->to_string() + ")"; }

LetNode::LetNode(std::vector<Env::kv_pair> && bd, node_ptr node, bool is_star) : bindings_(std::move(bd)), body_(node), star_(is_star) { }
node_ptr LetNode::eval(Env & env ) {
	Env current = env; // Seed new environment
	for (auto const & binding : bindings_) {
		current.insert(binding.first, binding.second->eval(star_ ? current : env), false);
	}
	return body_->eval(current);
}
std::string LetNode::to_string() const
{
	std::string out =  std::format("#<Let{}> (", star_ ? "*" : "");
	bool first = true;
	for (auto const & pr : bindings_) {
		if (!first) { out += ", "; }
		out += "(" + pr.first + ", " + pr.second->to_string() + ")";
		first = false;
	}
	return out + ")";
}

// Procedures

ProcNode::ProcNode(node_list && seq) : nodes_(std::move(seq)) { }
node_ptr ProcNode::eval(Env & env) {
	node_list args;
	node_ptr proc(nodes_.front()->eval(env));
	std::transform(std::next(nodes_.cbegin()), nodes_.cend(), std::back_inserter(args), [&env](const auto & node){
		return node->eval(env);
	});
	return proc->call(args);
}
std::string ProcNode::to_string() const
{
	std::string out = "#<Proc>[ ";
	bool first = true;
	for (const auto & node : nodes_) {
		if (!first) { out += ", "; }
		out += node->to_string();
		first = false; 
	}
	return out + " ]";
}

BuiltinNode::BuiltinNode(const std::string fname, const BuiltinNode::builtin_fxn & func) : name_(fname), fxn_(func) { }
node_ptr BuiltinNode::eval(Env&) { return shared_from_this(); }
node_ptr BuiltinNode::call(node_list & args) { return fxn_(args); }
std::string BuiltinNode::to_string() const { return std::string("#<Builtin>: ") + name_; }

LambdaNode::LambdaNode(std::vector<std::string> && arg_list, node_ptr body, std::string name) : arg_list_(std::move(arg_list)), body_(body), name_(name) { }
node_ptr LambdaNode::eval(Env & env)
{
	env_ = env; // Capture the environment at the time of constuction
	return shared_from_this();
}
node_ptr LambdaNode::call(node_list & args)
{
	if (args.size() != arg_list_.size()) {
		throw_error(std::format("runtime: lambda function requires {} args; called with {}", arg_list_.size(), args.size()));
	}

	// Add arguments into environment
	Env current = env_;
	auto l = arg_list_.cbegin();
	auto r = args.cbegin();
	while (l != arg_list_.cend()) {
		current.insert(*l, *r, false);
		++l; ++r;
	}

	// Add function itself into the environment (to allow for recursion).
	// We can just hand over the pointer, since this is a copy of
	// the environment (not attached to the object itself).
	if (!name_.empty()) {
		current.insert(name_, shared_from_this(), false);
	}

	// Eval
	return body_->eval(current);
}
std::string LambdaNode::to_string() const {
	std::ostringstream al;
	std::ostream_iterator<std::string> it(al, " ");
	std::copy(arg_list_.cbegin(), arg_list_.cend(), it);
	return std::format("#<Lambda>: [{}] ( ", name_) + al.str() + ") ";
}

// Check if a node can be interpreted as a valid list.
bool is_list(node_ptr node) {
	if (node->is_unit())  { return true;  }
	if (!node->is_pair()) { return false; }
	return is_list(node->get(1));
};

PairNode::PairNode(node_ptr l, node_ptr r) : first_(l), second_(r) { }
node_ptr PairNode::eval(Env & env )
	{ return std::make_shared<PairNode>(first_->eval(env), second_->eval(env)); }
node_ptr PairNode::get(std::size_t idx) const { return idx == 0 ? first_ : second_; }
std::string PairNode::to_string() const { return to_string_internal(); }
std::string PairNode::to_string_internal(bool outer) const
{
	// Logic for representing pairs and lists accurately.
	bool il = is_list(get(1));
	std::string out = "";
	if (!il || outer) { out += "("; }
	out += first_->is_pair()
	    ? std::dynamic_pointer_cast<PairNode>(first_)->to_string_internal(true)
	    : first_->to_string();
	if (!second_->is_unit()) {
		out += il ? " " : " . ";
		out += second_->is_pair()
			? std::dynamic_pointer_cast<PairNode>(second_)->to_string_internal(false)
			: second_->to_string();
	}
	if (!il || outer) { out += ")"; }
	return out;
}

CondNode::CondNode(node_list && p_seq, node_list && n_seq) : predicate_seq_(p_seq), node_seq_(n_seq) { assert(p_seq.size() == n_seq.size()); }
node_ptr CondNode::eval(Env & env)
{
	auto l = predicate_seq_.cbegin();
	auto r = node_seq_.cbegin();
	while (l != predicate_seq_.cend()) {
		if ((*l)->eval(env)->get_boolean()) {
			return (*r)->eval(env);
		}
		++l; ++r;
	}

	return std::make_unique<NullNode>();
}
std::string CondNode::to_string() const {
	std::string out = "#<Cond>";

	auto l = predicate_seq_.cbegin();
	auto r = node_seq_.cbegin();
	bool first = true;
	while (l != predicate_seq_.cend()) {
		if (!first) { out += ", "; }
		out += "(" + (*l)->to_string() + ", " + (*r)->to_string() + ")";
		first = false;
		++l; ++r;
	}

	return out;
}

AndNode::AndNode(node_list && n_seq) : nodes_(n_seq) { }
node_ptr AndNode::eval(Env & env)
{
	if (nodes_.size() == 0) { return std::make_unique<BoolNode>(true); }
	
	node_ptr val = nullptr;
	for (const auto & node: nodes_) {
		val = node->eval(env);
		if (!val->get_boolean()) { break; }
	}

	return val;
}
std::string AndNode::to_string() const {
	std::string out = "#<And>[ ";
	bool first = true;
	for (const auto & node : nodes_) {
		if (!first) { out += ", "; }
		out += node->to_string(); 
		first = false;
	}
	return out + " ]";
}

OrNode::OrNode(node_list && n_seq) : nodes_(n_seq) { }
node_ptr OrNode::eval(Env & env)
{
	for (const auto & node: nodes_) {
		node_ptr val = node->eval(env);
		if (val->get_boolean()) { return val; }
	}

	return std::make_unique<BoolNode>(false);;
}
std::string OrNode::to_string() const {
	std::string out = "#<Or>[ ";
	bool first = true;
	for (const auto & node : nodes_) {
		if (!first) { out += ", "; }
		out += node->to_string(); 
		first = false;
	}
	return out + " ]";
}

}

}