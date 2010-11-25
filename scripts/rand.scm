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
;; to gnubik-rotate-animated. The three random components are the face number
;; (0, 1 or 2), the slice number (-(size-1), ..., size-3, size-1), and the
;; direction to turn the slice (0 or 1).

(define (gnubik-randomize num)
  (let ((size (caddar (gnubik-cube-state))))
    (gnubik-rotate-animated
     (let loop ((num num) (ret '()))
       (if (eq? num 0)
           ret
           (loop (- num 1)
                 (cons (list (random 3)
                             (- (* 2 (random size)) (- size 1))
                             (random 2))
                       ret)))))))
  


;; Invoke the above procedure from a good few menu items for the convenience of
;; the user.


(define rand-menu (gnubik-create-menu (_ "_Randomize")))
(define animate-menu (gnubik-create-menu (_ "_Animated") rand-menu))
(define fast-menu (gnubik-create-menu (_ "_Fast") rand-menu))

(gnubik-register-script "_8" '(gnubik-randomize 8) animate-menu)
(gnubik-register-script "_7" '(gnubik-randomize 7) animate-menu)
(gnubik-register-script "_6" '(gnubik-randomize 6) animate-menu)
(gnubik-register-script "_5" '(gnubik-randomize 5) animate-menu)
(gnubik-register-script "_4" '(gnubik-randomize 4) animate-menu)
(gnubik-register-script "_3" '(gnubik-randomize 3) animate-menu)
(gnubik-register-script "_2" '(gnubik-randomize 2) animate-menu)
(gnubik-register-script "_1" '(gnubik-randomize 1) animate-menu)

(gnubik-register-script "_128"  '(begin (gnubik-randomize 128) (gnubik-run-moves)) fast-menu)
(gnubik-register-script "_64"  '(begin (gnubik-randomize 64) (gnubik-run-moves)) fast-menu)
(gnubik-register-script "_32"  '(begin (gnubik-randomize 32) (gnubik-run-moves)) fast-menu)
(gnubik-register-script "1_6"  '(begin (gnubik-randomize 16) (gnubik-run-moves)) fast-menu)
(gnubik-register-script "_8"  '(begin (gnubik-randomize 8) (gnubik-run-moves)) fast-menu)
(gnubik-register-script "_4"  '(begin (gnubik-randomize 4) (gnubik-run-moves)) fast-menu)
(gnubik-register-script "_2"  '(begin (gnubik-randomize 2) (gnubik-run-moves)) fast-menu)
(gnubik-register-script "_1"  '(begin (gnubik-randomize 1) (gnubik-run-moves)) fast-menu)

