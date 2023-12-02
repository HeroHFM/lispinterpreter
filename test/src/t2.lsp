(display (boolean? #t)) (newline)
(display (boolean? #f)) (newline)
(display (boolean? 1)) (newline)
(display (boolean? ())) (newline)
(display (boolean? (cons #t #f))) (newline) (newline)

(display (integer? 1)) (newline)
(display (integer? (list 1))) (newline)
(display (integer? ())) (newline) (newline)

(display (pair? (cons 1 2))) (newline)
(display (pair? (list 1 2))) (newline)
(display (pair? 1)) (newline)
(display (pair? ())) (newline) (newline)

(display (list? ())) (newline)
(display (list? (list 1))) (newline)
(display (list? (cons 1 2))) (newline) (newline)

(display (procedure? ())) (newline)
(display (procedure? +)) (newline)
(display (procedure? (lambda () 1))) (newline) (newline)

(display (null? ())) (newline)
(display (null? 1)) (newline)
(display (null? (begin)))