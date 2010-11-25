;;   Copyright (c) 2004 Dale Mellor
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




(define (gnubik-dump-state)
  (let ((cube (gnubik-cube-state)))
    
    (display "Cube geometry (version dimensionality size-of-dim-1 ...): ")
    (newline)(display "          ")(display (car cube))
    (newline)
    
    (array-for-each
     (lambda (a)
       (array-for-each
        (lambda (b)
          (display b)(display " "))
        a)
       (newline))
     (cdr cube))
    (newline)))





(define (debug-move-animated face)
  (let ((cube (gnubik-cube-state)))
    (if (check-cube-structure cube)
        (begin
          (clear-move-buffer!)
          (move-face cube face)
          (execute-move-buffer!)))))

(define _ gettext)

(define dbg-menu (gnubik-create-menu (_ "_Debug")))

(define menu (gnubik-create-menu (_ "_Move") dbg-menu))

(gnubik-register-script (_ "_Dump state") '(gnubik-dump-state) dbg-menu)

(gnubik-register-script "_F" '(debug-move-animated "f") menu)
(gnubik-register-script "_B" '(debug-move-animated "b") menu)
(gnubik-register-script "_L" '(debug-move-animated "l") menu)
(gnubik-register-script "_R" '(debug-move-animated "r") menu)
(gnubik-register-script "_U" '(debug-move-animated "u") menu)
(gnubik-register-script "_D" '(debug-move-animated "d") menu)
