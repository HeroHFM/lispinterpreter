#include "li/parse.hpp"
#include "li/utility.hpp"

#include <iostream>
#include <vector>
#include <cctype>
#include <memory>
#include <list>

namespace lisp {

namespace interpreter {

Parser::Parser(bool ml) : multiline_(ml) { }

void Parser::reset() { paren_ = 0; tokens_.clear(); }

void report_error(const char * text)
    { std::cout << "error: " << text << std::endl; }

Parser::status
Parser::tokenize(std::istream & src)
{
    std::string token = "";

    // If the token buffer contains a token, push it.
    auto try_push = [this, &token](){
        if (!token.empty()) {
            // Convert to lower case
            std::transform(token.begin(), token.end(), token.begin(), [](unsigned char chr){
                return std::tolower(chr);
            });
            tokens_.push_back(token);
            token.erase();
        }
    };

    // Parse tokens, one character at a
    // time from the input stream.

    char c;

    while (src.good()) {
        switch (c = src.get()) {
            case ';': // Comment
                try_push();
                while (src.good() && (src.get() != '\n'));
                break;
            case '(': // Open paren
                try_push();
                tokens_.push_back("(");
                ++paren_;
                break;
            case ')': // Close paren
                try_push();
                if (paren_ <= 0) {
                    report_error("tokenizer: unable to match `)` to any previous `(`");
                    return status::failure;
                }
                tokens_.push_back(")");
                --paren_;
                break;
            default: // Either part of a token or a space
                if (std::isspace(c)) { try_push(); }
                else if (c != EOF) { token += c; }
        }
    }

    try_push();

    // We either now have a complete or an incomplete
    // s-expression stored in the token list.
    return (paren_ ==  0) ?  status::success : status::incomplete;
}

// Find matching `)` given iterator to initial `(`.
// If not initial `(`, return begin + 1.
// If not found, exception is thrown.
std::vector<std::string>::const_iterator
find_match(const std::vector<std::string>::const_iterator & begin,
           const std::vector<std::string>::const_iterator & end)
{
    if (*begin != "(") { return begin + 1; };

    std::size_t paren = 1;
    auto it = begin + 1;
    
    for (; it != end; ++it) {
        if      (*it == "(") { ++paren; }
        else if (*it == ")") { --paren; }
        if (paren == 0) { break; }
    }

    if (paren != 0) {
        throw_error("parser: could not match `(` during immediate parsing ");
    }

    return it;
}

// Get iterators to each element of sequence, at a given level of the tree.
// This is needed because our sequence of tokens is linear (not nested),
// even though it represents a tree conceptually, and will be parsed into one.
// Note that each adjacent pair denotes an element's begin and end iterators.
std::vector<std::vector<std::string>::const_iterator>
split_level(const std::vector<std::string>::const_iterator & begin,
            const std::vector<std::string>::const_iterator & end)
{
    std::vector<std::vector<std::string>::const_iterator> indices;

    if ((*begin != "(") || (*(end - 1) != ")")) {
        throw_error("parser: could not parse s-expression");
    }

    // Scan for beginnings of elements
    if (begin + 1 != end - 1) {
        auto it = begin + 1;
        while (*it != ")") {
            indices.push_back(it);
            if (*it == "(") {
                auto next = find_match(it, end) + 1;
                it = next;
            } else {
                ++it;
            }
        }
    }

    // One-past-the-end of last element (if any)
    indices.push_back(end - 1);

    return indices;
}

bool is_bool(std::string const & s) { return (s == "#t") || (s == "#f"); }
bool is_int(std::string const & s)
{
    // Not ideal, but ensures the parsing is consistent.
    try { std::stoi(s); return true; }
    catch (std::invalid_argument const &) { return false; }
    catch (std::out_of_range const &) { return true; }
}
bool is_identifier(std::string const & s) { return !is_bool(s) && !is_int(s); }

// Nest vector elements into a lisp list.
std::unique_ptr<ASTNode>
construct_list(std::list<std::unique_ptr<ASTNode>> & lst)
{
    // Base case
    if (lst.size() == 0) { return std::make_unique<UnitNode>(); }

    node_ptr elem = std::move(lst.front()); 
    lst.pop_front();
    
    // Nest
    return std::make_unique<PairNode>(
        std::move(elem),
        construct_list(lst)
    );
}

// Main element of parser. Parses ranges of from token list into nodes.
std::unique_ptr<ASTNode>
parse_immediate(const std::vector<std::string>::const_iterator & begin,
                const std::vector<std::string>::const_iterator & end)
{
    if (begin == end) { throw_error("parser: nothing to parse"); }

    if (*begin == "(") {
        // s-expression

        if (*(end - 1) != ")")
            { throw_error("parser: encountered malformed s-expression"); }

        // Unit
        if (begin + 1 == end - 1) { return std::make_unique<UnitNode>(); }

        // Split into individual elements
        auto level = split_level(begin, end);

        // Note that a size of 1 represents 0 complete elements.
        if (level.size() <= 1)
            { throw_error("parser: failed to split s-expression"); }

        // Cons
        if (*level[0] == "cons") {
            if (level.size() != 4) { throw_error("cons: illegal syntax"); }
            return std::make_unique<PairNode>(
                parse_immediate(level[1], level[2]),
                parse_immediate(level[2], level[3])
            );
        }

        // List
        if (*level[0] == "list") {
            // Generate the nodes
            std::list<std::unique_ptr<ASTNode>> nodes;
            std::size_t l_ind = 1, r_ind = 2;
            for (; r_ind < level.size(); ++l_ind, ++r_ind) {
                nodes.emplace_back(parse_immediate(level[l_ind], level[r_ind]));
            }
            // Construct a list
            return construct_list(nodes);
        }

        // If
        if (*level[0] == "if") {
            if (level.size() != 5) { throw_error("if: illegal syntax"); }
            // Re-use CondNode for ifs as well.
            return std::make_unique<CondNode>(
                // Predicates
                ASTNode::node_list { parse_immediate(level[1], level[2]),
                    std::make_unique<BoolNode>(true) },
                // Bodies
                ASTNode::node_list { parse_immediate(level[2], level[3]),
                  parse_immediate(level[3], level[4]) }
            );
        }

        // Cond
        if (*level[0] == "cond") {
            ASTNode::node_list pred;
            ASTNode::node_list seq;

            std::size_t l_ind = 1, r_ind = 2;
            for (; r_ind < level.size(); ++l_ind, ++r_ind) {
                auto pair = split_level(level[l_ind], level[r_ind]);
                if (pair.size() != 3) { throw_error("cond: illegal condition list"); }
                
                pred.emplace_back(parse_immediate(pair[0], pair[1]));
                seq.emplace_back(parse_immediate(pair[1], pair[2]));
            }

            return std::make_unique<CondNode>(std::move(pred), std::move(seq));
        }

        // Define
        if (*level[0] == "define") {
            if (level.size() != 4) { throw_error("define: illegal syntax"); }

            // Special function definition syntax
            if (*level[1] == "(") {
                std::vector<std::string> arg_list;

                auto arg_iter = split_level(level[1], level[2]);

                if (arg_iter.size() < 2) { throw_error("lambda: illegal argument list"); }

                std::size_t l_ind = 1, r_ind = 2;
                for (; r_ind < arg_iter.size(); ++l_ind, ++r_ind) {
                    // Disallow nesting and non-identifiers
                    if ((std::distance(arg_iter[l_ind], arg_iter[r_ind]) != 1)
                        || !is_identifier(*arg_iter[l_ind]))
                        { throw_error("lambda: illegal argument list"); }
                    arg_list.push_back(*arg_iter[l_ind]);
                }

                // Materialize
                return std::make_unique<BindNode>( *arg_iter[0],
                    std::make_unique<LambdaNode>(std::move(arg_list), parse_immediate(level[2], level[3]), *arg_iter[0])
                );

            // Binding a regular variable identifier
            } else {
                if (!is_identifier(*level[1])) { throw_error("define: illegal syntax"); }
                return std::make_unique<BindNode>(*level[1], parse_immediate(level[2], level[3]));
            }
        }

        // Let[*]
        if ((*level[0] == "let") || *level[0] == "let*") {
            if (level.size() < 4) { throw_error("let: illegal syntax"); }
            
            // Extract the bindings
            std::vector<Env::kv_pair> bindings;

            auto pair_iter = split_level(level[1], level[2]);

            std::size_t l_ind = 0, r_ind = 1;
                for (; r_ind < pair_iter.size(); ++l_ind, ++r_ind) {
                    auto pair = split_level(pair_iter[l_ind], pair_iter[r_ind]);
                    if (pair.size() != 3 || !is_identifier(*pair_iter[l_ind])) { throw_error("let: illegal binding list"); }
                    
                    bindings.emplace_back(*pair[0], parse_immediate(pair[1], pair[2]));
                }

            // Extract the expression sequence
            ASTNode::node_list nodes;

            l_ind = 2, r_ind = 3;
            for (; r_ind < level.size(); ++l_ind, ++r_ind) {
                nodes.emplace_back(parse_immediate(level[l_ind], level[r_ind]));
            }

            // Materialize
            return std::make_unique<LetNode>(
                std::move(bindings),
                std::make_unique<SeqNode>(std::move(nodes)),
                *level[0] == "let*"
            );
        }
        
        // Lambda
        if (*level[0] == "lambda") {
            if (level.size() != 4) { throw_error("lambda: illegal syntax"); }
            
            // Parse argument list
            std::vector<std::string> arg_list;

            auto arg_iter = split_level(level[1], level[2]);

            std::size_t l_ind = 0, r_ind = 1;
            for (; r_ind < arg_iter.size(); ++l_ind, ++r_ind) {
                // Disallow nesting and non-identifiers
                if ((std::distance(arg_iter[l_ind], arg_iter[r_ind]) != 1)
                    || !is_identifier(*arg_iter[l_ind]))
                    { throw_error("lambda: illegal argument list"); }
                arg_list.push_back(*arg_iter[l_ind]);
            }

            return std::make_unique<LambdaNode>(std::move(arg_list), parse_immediate(level[2], level[3]));
        }

        // Procedure Call / Sequence / And / Or
        
        // All sequence nodes have roughly the same structure.

        ASTNode::node_list nodes;
        
        std::size_t l_ind = 0, r_ind = 1;
        if (*level[0] == "begin" // Skip keyword
            || *level[0] == "and"
            || *level[0] == "or") { ++l_ind; ++r_ind; }
        
        for (; r_ind < level.size(); ++l_ind, ++r_ind) {
            nodes.emplace_back(parse_immediate(level[l_ind], level[r_ind]));
        }

        if (*level[0] == "begin")
            { return std::make_unique<SeqNode>(std::move(nodes)); }
        else if (*level[0] == "and")
            { return std::make_unique<AndNode>(std::move(nodes)); }
        else if (*level[0] == "or")
            { return std::make_unique<OrNode>(std::move(nodes)); }
        else
            { return std::make_unique<ProcNode>(std::move(nodes)); } 
        
    } else {
        // literal or identifier
        if (end - begin != 1) { throw_error("parser: invalid s-expression"); }

        // bool
        if (*begin == "#t") { return std::make_unique<BoolNode>(true);  }
        if (*begin == "#f") { return std::make_unique<BoolNode>(false); }

        // integer
        try { return std::make_unique<IntNode>(std::stoi(*begin)); }
        catch (std::invalid_argument const &) { /* not an int */ }
        catch (std::out_of_range const &) { throw_error("parser: integer too large"); }

        // identifier
        return std::make_unique<VarNode>(*begin);
    }
}

// Precondition: SeqNode is empty.
Parser::status
Parser::parse(std::istream & src, SeqNode & dst)
{

    status result = tokenize(src);

    if (result == status::incomplete && !multiline_) {
        throw_error("parser: input does not form a valid expression");
    }

    if (result != status::success) { return result; }
    if (tokens_.empty()) { return status::success; }

    // Note:
    // I am using a mix of a custom error return type and
    // exceptions here in order to keep exceptions out of
    // the critical path of the parsing mechanism.

    try {
        dst.sequence_.emplace_front(
            parse_immediate(tokens_.cbegin(), tokens_.cend())
        );
        return status::success;
    } catch (std::string const & e) {
        throw; // If we have an error here, we can handle it in main.
    } catch (...) {
        throw_error("immediate parsing failed");
    }

    return status::failure;

}

}

}
