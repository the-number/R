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


(gnubik-register-script "_Debug/_Dump state" "gnubik-dump-state")



(define (debug-move-animated face)
  (let ((cube (gnubik-cube-state)))
    (if (check-cube-structure cube)
        (begin
          (clear-move-buffer!)
          (move-face cube face)
          (execute-move-buffer!)))))

(gnubik-register-script "_Debug/_Move/_F" "debug-move-animated \"f\"")
(gnubik-register-script "_Debug/_Move/_B" "debug-move-animated \"b\"")
(gnubik-register-script "_Debug/_Move/_L" "debug-move-animated \"l\"")
(gnubik-register-script "_Debug/_Move/_R" "debug-move-animated \"r\"")
(gnubik-register-script "_Debug/_Move/_U" "debug-move-animated \"u\"")
(gnubik-register-script "_Debug/_Move/_D" "debug-move-animated \"d\"")
