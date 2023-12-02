# Lisp Interpreter & REPL

## Build & Install

Let $TOP_DIR denote the directory containing this README file.
Let $INSTALL_DIR denote the directory into which this software is to be installed.

To build and install the software, use the following commands:
```sh
$ cd $TOP_DIR
$ cmake -H. -Btmp_cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR
$ cmake --build tmp_cmake --clean-first --target install
```

## Run demo

To run a demonstration, use the following command:
```sh
$ $INSTALL_DIR/bin/demo
```

The demo runs some of the example programs provided in the `$INSTALL_DIR/bin/examples` folder.

## Using The Interpreter And REPL

To run the interpreter in interactive mode (REPL):
```sh
$ $INSTALL_DIR/bin/lisp
```

To run the interpreter on a source file:
```sh
$ $INSTALL_DIR/bin/lisp filename.lsp
```

OR

```sh
$ $INSTALL_DIR/bin/lisp < filename.lsp
```

Note that there is a slight difference in how the REPL and interpreter parse files. In a file, it is fine to have s-expressions like `() ()`, however this is not so for the repl (it must be a single element or expression per line, not multiple).

## rlwrap

The `rlwrap` package is already installed on the lab machines, and can make text entry into the interpreter much more convenient. Without it, it is not possible to use the arrow keys to move around text that has already been typed.

However, it can sometimes behave unexpectedly, and has not been fully tested with this software.

```sh
$ rlwrap $INSTALL_DIR/bin/lisp
```

## Language Quick Reference

## Important note.
There was a slight typo in the definition of `cond` in the report, so please refer to the one that follows instead.

-- Comments --

Line comments start with a semicolon (;).


-- Booleans --

#t => true
#f => false


-- Integers --

[+-]?[0-9]+ => integer

(* x y ... z)   => x * y * ... * z
(+ x y ... z)   => x + y + ... + z
(- x y ... z)   => x - y - ... - z
(/ x y ... z)   => a / y / ... / z

(= x y ... z)   => #t if x = y = ... = z else #f
(< x y ... z)   => #t if x < y < ... < z else #f
(> x y ... z)   => #t if x > y > ... > z else #f
(<= x y ... z)  => #t if x <= y <= ... <= z else #f
(>= x y ... z)  => #t if x >= y >= ... >= z else #f

(abs x)         => -x if x < 0 else x
(expt x y)      => x ^ y

(max x y ... z) => max(x, y, ..., z)
(min x y ... z) => min(x, y, ..., z)
(modulo x y)    => x % y

(zero? x)       => #t if x is 0 else #f


-- Pairs --

(cons a b)       => a pair (a . b)
(car (cons a b)) => a
(cdr (cons a b)) => b


-- Lists --

(list a b ... z)     => Create list (a b ... z)
(null? list)         => #t if list is () else #f
(length lst)         => Length of the list lst
(append lst1 lst2)   => Concatenation of lists lst1 and lst2


-- Procedures --

(lambda (arg1 arg2 ... argN) body)
        => Procedure with n arguments. When called, the body
           will be evaluated with arg1 to argN bound to the
           values the lambda was called with.  


-- Variables / Definitions --

(define name value) => Define variable in the top level
(define (fun arg1 arg2 ... argN) body)
        => Equivalent to (define fun (lambda (arg1 arg2 ... argN) body))

(let ((v1 bexp1) (v2 bexp2) ...) exp1 exp2 ... expN)
        => Run exp1, exp2, ..., expN in an environment with v1 bound to the value of bexp1, etc. Returns value of expN.

(let* ((v1 bexp1) (v2 bexp2) ... ) exp1 exp2 ... expN)
       => Like let, but expression bexpN is evaluated in an environment where v1 to vN-1 are already bound to the values of bexp1 to bexpN-1 respectively (the bindings are happening in turn, as if they were nested, rather than happening all at once).


-- Control Flow --

(if test expr1 expr2)
    => Evaluate test. Then if test is #f evaluate expr2,
       otherwise evaluate expr1.
(cond (t1 e1) (t2 e2) ... (tN eN))
    => Test each t1 to tN from left to right. Evaluate the first
       ei for which ti is not #f (if any).
(and x y ... z)
    => Evaluate from left to right. Return value of first
       expression which returns #f. If none return #f,
       return last. If no expressions, return #t.
(or x y ... z) 
    => Evaluate from left to right. Return value of first
    experssion which returns not #f. Otherwise, return #f.
(not x)
    => Return #t if x is #f, otherwise #f.


-- Other builtin functions --

(boolean? expr)   => #t if expr is of type boolean, #f otherwise
(integer? expr)   => #t if expr is of type integer, #f otherwise
(pair? expr)      => #t if expr is of type pair, #f otherwise
(list? expr)      => #t if expr is of type list, #f otherwise
(procedure? expr) => #t if expr is of type procedure, #f otherwise

(begin exp1 exp2 ... expN) => Evaluates expressions from left to right, returns value of expN
(display x) => Prints out human-readable representation of x
(newline) => Prints new line