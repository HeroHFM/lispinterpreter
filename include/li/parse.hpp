#ifndef H_PARSE
#define H_PARSE

#include "li/ast.hpp"

#include <iostream>
#include <vector>
#include <string>

namespace lisp {

namespace interpreter {

class Parser {
public:
    Parser() = default;
    ~Parser() = default;

    Parser(bool);
    
    using token_list = std::vector<std::string>;

    enum status {
        success = 0,
        failure,
        incomplete,
    };

    void reset();
    status tokenize(std::istream & src);
    status parse(std::istream & src, SeqNode & dst);

private:
    std::size_t paren_ = 0;
    bool multiline_ = false;
    token_list tokens_;
};

}

}

#endif
