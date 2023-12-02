; Modified from https://wiki.c2.com/?FibonacciSequence
(define (fib n)
    (cond
      ((= n 0) 0)
      ((= n 1) 1)
      (#t
        (+ (fib (- n 1))
           (fib (- n 2))))))

(display (fib 1)) (newline)
(display (fib 2)) (newline)
(display (fib 3)) (newline)
(display (fib 4)) (newline)
(display (fib 5)) (newline)
(display (fib 6)) (newline)
(display (fib 7)) (newline)
