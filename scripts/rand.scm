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

(gnubik-register-script "_Randomize/_Animated/_8"
                        "gnubik-randomize 8")
(gnubik-register-script "_Randomize/_Animated/_7"
                        "gnubik-randomize 7")
(gnubik-register-script "_Randomize/_Animated/_6"
                        "gnubik-randomize 6")
(gnubik-register-script "_Randomize/_Animated/_5"
                        "gnubik-randomize 5")
(gnubik-register-script "_Randomize/_Animated/_4"
                        "gnubik-randomize 4")
(gnubik-register-script "_Randomize/_Animated/_3"
                        "gnubik-randomize 3")
(gnubik-register-script "_Randomize/_Animated/_2"
                        "gnubik-randomize 2")
(gnubik-register-script "_Randomize/_Animated/_1"
                        "gnubik-randomize 1")

(gnubik-register-script "_Randomize/_Fast/128"
                        "begin (gnubik-randomize 128) (gnubik-run-moves)")
(gnubik-register-script "_Randomize/_Fast/64"
                        "begin (gnubik-randomize 64) (gnubik-run-moves)")
(gnubik-register-script "_Randomize/_Fast/32"
                        "begin (gnubik-randomize 32) (gnubik-run-moves)")
(gnubik-register-script "_Randomize/_Fast/16"
                        "begin (gnubik-randomize 16) (gnubik-run-moves)")
(gnubik-register-script "_Randomize/_Fast/8"
                        "begin (gnubik-randomize 8) (gnubik-run-moves)")
(gnubik-register-script "_Randomize/_Fast/4"
                        "begin (gnubik-randomize 4) (gnubik-run-moves)")
(gnubik-register-script "_Randomize/_Fast/2"
                        "begin (gnubik-randomize 2) (gnubik-run-moves)")
(gnubik-register-script "_Randomize/_Fast/1"
                        "begin (gnubik-randomize 1) (gnubik-run-moves)")
