#ifndef H_AST
#define H_AST

#include "li/env.hpp"

#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace lisp {

namespace interpreter {

class ASTNode : public std::enable_shared_from_this<ASTNode> {
public:
    using node_ptr = std::shared_ptr<ASTNode>;
    using node_list = std::list<node_ptr>;

    virtual node_ptr eval(Env & env) = 0;
    virtual std::string to_string() const = 0;
    virtual ~ASTNode() = default;

    // Numeric types
    virtual bool is_numeric() const { return false; }
    virtual int get_numeric() const;

    // Boolean types
    virtual bool is_boolean() const { return false; }
    virtual bool get_boolean() const;

    // Procedure types
    virtual bool is_callable() const { return false; }
    virtual node_ptr call(node_list &);

    // Pair types
    virtual bool is_pair() const { return false; }
    virtual node_ptr get(std::size_t) const;

    // Unit/Null types
    virtual bool is_unit() const { return false; }
    virtual bool is_null() const { return false; }

    // Identifier types
    virtual bool is_var() const { return false; }
    virtual std::string get_identifier() const;

    friend std::ostream& operator<<(std::ostream & os, const ASTNode & node);
};

class IntNode : public ASTNode {
public:
    IntNode(int);
    node_ptr eval(Env & env);
    std::string to_string() const;

    bool is_numeric() const override { return true; }
    int get_numeric() const override;

private:
    const int value_;
};

class BoolNode : public ASTNode {
public:
    BoolNode(bool);
    node_ptr eval(Env & env);
    std::string to_string() const;

    bool is_boolean() const override { return true; }
    bool get_boolean() const override;

private:
    const bool value_;
};

class UnitNode : public ASTNode {
public:
    UnitNode() = default;
    node_ptr eval(Env & env);
    bool is_unit() const override { return true; }
    std::string to_string() const;
};

class NullNode : public ASTNode {
public:
    NullNode() = default;
    NullNode(std::string msg);
    node_ptr eval(Env & /* env */);
    bool is_null() const override { return true; }
    std::string to_string() const;
private:
    std::string msg_ = "";
};

class SeqNode : public ASTNode {
public:
    SeqNode() = default;
    SeqNode(node_list && seq);
    node_ptr eval(Env & env);
    std::string to_string() const;

    node_list sequence_;
};

class VarNode : public ASTNode {
public:
    VarNode(std::string name);
    node_ptr eval(Env & env);
    std::string to_string() const;

    bool is_var() const override { return true; }
    std::string get_identifier() const override;

private:
    std::string name_;
};

class BindNode : public ASTNode {
public:
    BindNode(std::string name, node_ptr value);
    node_ptr eval(Env & env);
    std::string to_string() const;

private:
    std::string name_;
    node_ptr value_;
};

class LetNode : public ASTNode {
public:
    LetNode(std::vector<Env::kv_pair> && bd, node_ptr node, bool star = false);
    node_ptr eval(Env & env);
    std::string to_string() const;

private:
    std::vector<Env::kv_pair> bindings_;
    node_ptr body_;
    bool star_;
};

// Calling a procedure
class ProcNode : public ASTNode {
public:
    ProcNode(node_list && nodes);
    node_ptr eval(Env & env);
    std::string to_string() const;

private:
    node_list nodes_;
};

class BuiltinNode : public ASTNode {
public:
    using builtin_fxn = std::function<node_ptr(node_list &)>;

    BuiltinNode(const std::string, const builtin_fxn &);
    node_ptr eval(Env & env);
    node_ptr call(node_list &) override;
    std::string to_string() const;

    bool is_callable() const override { return true; }

private:
    const std::string name_;
    const builtin_fxn fxn_;
};

class LambdaNode : public ASTNode {
public:
    LambdaNode(std::vector<std::string> && arg_list, node_ptr body, std::string name = "");
    node_ptr eval(Env & env);
    node_ptr call(node_list &) override;
    std::string to_string() const;

    bool is_callable() const override { return true; }

private:
    const std::vector<std::string> arg_list_;
    node_ptr body_;
    Env env_;
    std::string name_;
};

class PairNode : public ASTNode {
public:
    PairNode(node_ptr l, node_ptr r);
    node_ptr eval(Env & env);
    std::string to_string() const;

    bool is_pair() const override { return true; }
    node_ptr get(std::size_t) const override;
protected:
    // Support special printing of lists
    std::string to_string_internal(bool outer = true) const;
private:
    node_ptr first_;
    node_ptr second_;
};

class CondNode : public ASTNode {
public:
    CondNode(node_list && p_seq, node_list && n_seq);
    node_ptr eval(Env & env);
    std::string to_string() const;

private:
    node_list predicate_seq_;
    node_list node_seq_; 
};

class AndNode : public ASTNode {
public:
    AndNode(node_list && nodes);
    node_ptr eval(Env & env);
    std::string to_string() const;

private:
    node_list nodes_;
};

class OrNode : public ASTNode {
public:
    OrNode(node_list && nodes);
    node_ptr eval(Env & env);
    std::string to_string() const;

private:
    node_list nodes_;
};

bool is_list(node_ptr node);

}

}

#endif
