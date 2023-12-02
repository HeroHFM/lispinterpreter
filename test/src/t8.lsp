(define d display)(define n newline)
(d ((lambda () 1)))(n) ; Call a lambda with no parameters
(d ((lambda (x) (+ x x)) 5))(n)
(d (define (square x) (expt x 2)))(n)
(d (square 3))(n)
(d (let ((nine (square 3))) nine))(n)
(d (let* ((four (square 2)) (sixteen (square four))) sixteen))(n)
(d (let ((x 1)) (let ((x 2)) x)))(n)