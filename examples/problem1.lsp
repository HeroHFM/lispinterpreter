; Problem 1 - https://projecteuler.net/problem=1

(define (map f lst)
        (if (null? lst) () (cons (f (car lst)) (map f (cdr lst)))))

(define (filter p lst)
        (cond ((null? lst)   ())
              ((p (car lst)) (cons (car lst) (filter p (cdr lst))))
              (#t            (filter p (cdr lst)))))

(define (reduce f lst init)
        (if (null? lst) init 
        (f (car lst) (reduce f (cdr lst) init))))

; [start, end)
(define (range start end)
        (if (>= start end) () (cons start (range (+ start 1) end))))

(define (multiple_of_3_or_5 x) (or (zero? (modulo x 3)) (zero? (modulo x 5))))

; Solve:
(display (reduce + (filter multiple_of_3_or_5 (range 1 1000)) 0))
(newline)