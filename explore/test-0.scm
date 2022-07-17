;;   test-0 (C) 2022 Gunter Liszewski
;;
;;   This program is free software; you can redistribute it and/or modify
;;   it under the terms of the GNU General Public License as published by
;;   the Free Software Foundation; either version 3 of the License, or
;;   (at your option) any later version.
;;
;;   This program is distributed in the hope that it will be useful,
;;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;   GNU General Public License for more details.
;;
;;   You should have received a copy of the GNU General Public License
;;   along with this program.  If not, see <http://www.gnu.org/licenses/>.


(define (number->colour a)
  (case a
    ((0) "red    ")
    ((1) "green  ")
    ((2) "blue   ")
    ((3) "cyan   ")
    ((4) "magenta")
    ((5) "yellow ")
    (else "what!?")))

(define (test-dump-state)
  (let ((a (gnubik-cube-state)))
    
    (display "Cube geometry (version dimensionality size-of-dim-1 ...): ")
    (newline)(display "          ")(display (car a))
    (newline)
    
    (array-for-each
     (lambda (b)
       (array-for-each
        (lambda (c)
          (display (number->colour c))
	  (display " "))
        b)
       (newline))
     (cdr a))
    (newline)))





(define (test-move-animated a)
  (let ((b (gnubik-cube-state)))
    (if (check-cube-structure b)
        (begin
          (clear-move-buffer!)
          (move-face b a)
          (execute-move-buffer!)))))

>(define _ gettext)

(define test-menu (gnubik-create-menu (_ "_Test 0")))

(define menu (gnubik-create-menu (_ "_Move to test") test-menu))

(gnubik-register-script (_ "_Show state") '(test-dump-state) test-menu)

(gnubik-register-script "_Front" '(test-move-animated "f") menu)
(gnubik-register-script "_Back" '(test-move-animated "b") menu)
(gnubik-register-script "_Left" '(test-move-animated "l") menu)
(gnubik-register-script "_Right" '(test-move-animated "r") menu)
(gnubik-register-script "_Up" '(test-move-animated "u") menu)
(gnubik-register-script "_Down" '(test-move-animated "d") menu)
