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



;; Procedure which generates a list of lists of three random items, and sends it
;; to gnubik-append-moves. The three random components are the face number
;; (0, 1 or 2), the slice number (-(size-1), ..., size-3, size-1), and the
;; direction to turn the slice (0 or 1).

(define (gnubik-randomize num)
  (let ((size (caddar (gnubik-cube-state))))
    (gnubik-append-moves 
     (let loop ((num num) (ret '()))
       (if (eq? num 0)
           ret
           (loop (- num 1)
                 (cons (list (random 3)
                             (- (* 2 (random size)) (- size 1))
                             (random 2))
                       ret)))))))
  
(define _ gettext)

(define rand-menu (gnubik-create-menu (_ "_Randomize")))

(gnubik-register-script "_8" '(gnubik-randomize 8) rand-menu)
(gnubik-register-script "_7" '(gnubik-randomize 7) rand-menu)
(gnubik-register-script "_6" '(gnubik-randomize 6) rand-menu)
(gnubik-register-script "_5" '(gnubik-randomize 5) rand-menu)
(gnubik-register-script "_4" '(gnubik-randomize 4) rand-menu)
(gnubik-register-script "_3" '(gnubik-randomize 3) rand-menu)
(gnubik-register-script "_2" '(gnubik-randomize 2) rand-menu)
(gnubik-register-script "_1" '(gnubik-randomize 1) rand-menu)

