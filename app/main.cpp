#include "li/ast.hpp"
#include "li/env.hpp"
#include "li/parse.hpp"
#include "li/builtins.hpp"

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <format>

// #define DEBUG

// Constants
const char * prompt  = "\t";
const char * version = "V0.03a"; 

void print_usage()
    { std::cout << "USAGE: ./lisp [filename]" << std::endl; }
void print_version()
    { std::cout << "(lisp repl) " << version << std::endl; }

// Print error and exit the program (unrecoverable)
void panic(std::string const & error_string) {
    std::cout << "error: " << error_string << std::endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    // Check invocation
    if (argc > 2) { print_usage(); exit(EXIT_FAILURE); }

    // Construct an environment
    std::unordered_map<std::string, lisp::interpreter::ASTNode::node_ptr> top_level;
    lisp::interpreter::Builtins builtins; 
    lisp::interpreter::Env env(&top_level, &builtins.functions);

    using status = typename lisp::interpreter::Parser::status;

    if ((argc == 2) || (argc == 1 && !isatty(fileno(stdin)))) {
        // Read from file or file-like object
        std::stringstream ss;
        ss << "(begin ";
        try {
            if (argc == 2) {
                std::ifstream file(argv[1]);
                if (!file.is_open()) {
                    panic(std::format("could not open file: {}", argv[1]));
                }
                ss << file.rdbuf();
            } else {
                ss << std::cin.rdbuf();
            }
        } catch (std::exception const & e) {
            panic(std::format("reading from file: {}",  e.what()));
        } catch (...) {
            panic("failed to create an input stream");
        }
        ss << ")";

        try {
            // Parse the data
            lisp::interpreter::Parser parse(false); // No multiline
            lisp::interpreter::SeqNode program;
            parse.parse(ss, program);
            // Run
            lisp::interpreter::ASTNode::node_ptr value = program.eval(env);
            if (!value->is_null()) { std::cout << *value << std::endl; }
        }
        catch (std::string const & e)
            { std::cout << "error: " << e << std::endl; }
        catch (std::exception const & e)
            { std::cout << "error: " << e.what() << std::endl; }
        catch (char const * e)
            { std::cout << "error: " << e << std::endl; }
        catch (...)
            { std::cout << "error: " << "runtime: error" << std::endl; }
    } else {
        lisp::interpreter::Parser parse(true); // Allow multiline
        // Start REPL
        print_version();
        std::string line;
        std::stringstream ss;
        bool alive = true;
        while (alive) {
            try {
                parse.reset();
                status result = status::incomplete;
                lisp::interpreter::SeqNode program;

                // Read until a valid s-expression can be assembled, or
                // we know that one never will be.
                std::cout << prompt;
                while (result == status::incomplete) {
                    
                    std::getline(std::cin, line); // Get a line

#ifdef DEBUG
                    std::cout << "got line: " << line << std::endl;
#endif

                    // Escape if EOF reached.
                    if (std::cin.eof() || std::cin.fail())
                        { std::cout << std::endl; alive = false; break; }

                    // Load the stream
                    ss.clear(); ss << line;
                    // Parse
                    result = parse.parse(ss, program); 
                }
                
#ifdef DEBUG
                std::cout << program << std::endl;
#endif
                // Run
                lisp::interpreter::ASTNode::node_ptr value = program.eval(env);
                if (!value->to_string().empty()) { std::cout << *value << std::endl; }
            }
            catch (std::string const & e)
                { std::cout << "error: " << e << std::endl; }
            catch (std::exception const & e)
                { std::cout << "error: " << e.what() << std::endl; }
            catch (char const * e)
                { std::cout << "error: " << e << std::endl; }
            catch (...)
                { std::cout << "error: " << "runtime: error" << std::endl; }
        }
    }

    exit(EXIT_SUCCESS);
}
