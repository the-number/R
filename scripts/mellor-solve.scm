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

;; I'm not normally one for global variables, but this is so pervasive
;; throughout this unit I couldn't resist. It is the scheme reflection of
;; GNUbik's own cube object.

(define cube '())




;; Whenever we access the cube, or perform moves on it, we call a function which
;; performs symbolic rotations about the sides l, r, b and f. This way, we only
;; have to work out one quarter of the total moves to solve the cube, and the
;; rest come by symmetry. The following variable gets a function which performs
;; the rotations (it will be a function which takes a string in and gives a
;; transformed string back).

(define rotated-flubrd-symbol '())




;; Function to create the procedure that gets assigned to the variable binding
;; above, given the number of turns required. For example, if one turn is
;; requested, then any algorithms which are centred around the front face will,
;; unwittingly, be applying themselves to the right face.

(define (rotated-flubrd-symbol-set! turns)
  (set! rotated-flubrd-symbol
        (lambda (s)
          (do ((s (string-copy s))
               (j 0 (+ 1 j)))
              ((eq? j turns) s)
            (do ((i 0 (+ i 1))) ((eq? i (string-length s))
                                 s)
              (string-set! s i
                           (case (string-ref s i)
                             ((#\l) #\f)
                             ((#\f) #\r)
                             ((#\r) #\b)
                             ((#\b) #\l)
                             (else (string-ref s i)))))))))




;; So, if proc is designed to solve a block on the front face, we apply it to
;; four different rotations of the symbolic letters and have it solve a block on
;; four faces.

(define (repeat-for-all-rotations proc)
  (do ((i 0 (+ i 1))) ((eq? i 4))
    (rotated-flubrd-symbol-set! i)
    (proc)))




;; Wrapper around get-colour-symbolic which applies the cube rotation first,
;; i.e. it is influenced by the script later setting rotated-flubrd-symbol by
;; calling rotated-flubrd-symbol-set!

(define (lookup-colour s)
  (get-colour-symbolic cube (rotated-flubrd-symbol s)))




;; Wrapper around move-face which applies the cube rotation first. Note that
;; this takes a string consisting of multiple moves, each one represented by two
;; letters, e.g. "f+"; the single-letter symbols which move-face accepts cannot
;; be used here.

(define (do-moves moves)
  (do ((moves moves (substring moves 2))) ((string-null? moves))
    (move-face cube (rotated-flubrd-symbol (substring moves 0 2)))))
 



;; This introduces xsubstring, which allows us to perform cyclic rotations on
;; the characters of a string.

(use-modules ((srfi srfi-13)))




;; Think about fixing the uf block, but then apply the algorithms with the cube
;; symbolically rotated four times. Look for the required block in all possible
;; locations, and when it is found lookup the moves that are needed to bring it
;; to uf, without upsetting any of the other top edges.

(define (mellor-top-edge-solve)
  (repeat-for-all-rotations
   (lambda ()
     (let ((u (lookup-colour "u"))
           (f (lookup-colour "f")))
       (let trial ((moves '(("fu" . "f-u+l-u-")
                            
                            ("ur" . "r-u-r+u+")
                            ("ru" . "r-f-")
                            ("bu" . "b-u-r-u+")
                            ("ub" . "b-u-u-b+u-u-")
                            ("ul" . "l+u+l-u-")
                            ("lu" . "l+f+")
                            
                            ("fr" . "u-r+u+")
                            ("rf" . "f-")
                            ("rb" . "r+r+f-r-r-")
                            ("br" . "u-r-u+")
                            ("lb" . "l+l+f+l-l-")
                            ("bl" . "u+l+u-")
                            ("fl" . "u+l-u-")
                            ("lf" . "f+")
                            
                            ("fd" . "d+r+f-r-")
                            ("df" . "f+f+")
                            ("rd" . "r+f-r-")
                            ("dr" . "d-f+f+")
                            ("db" . "d+d+f+f+")
                            ("bd" . "d-r+f-r-")
                            ("dl" . "d+f+f+")
                            ("ld" . "l-f+l+"))))
         (if (not (null? moves))
             (if (and (eq? u (lookup-colour (caar moves)))
                      (eq? f (lookup-colour (xsubstring (caar moves) 1 3))))
                 (do-moves (cdar moves))
                 (trial (cdr moves)))))))))




;; Concentrate on getting the block ufr correct, but then apply the procedure to
;; all four sides of the cube. Look for the correct block in all locations, and
;; then lookup the moves required to get it to ufr.

(define (mellor-top-corner-solve)
  (repeat-for-all-rotations
   (lambda ()
     (let ((top-colour (lookup-colour "u"))
           (front-colour (lookup-colour "f"))
           (right-colour (lookup-colour "r")))
       (let loop ((moves '(("rfd" .   "r-d-r+")
                           ("fld" . "d+r-d-r+")
                           ("lbd" . "f+d-d-f-")
                           ("brd" . "f+d-f-")
                           
                           ("fdr" .   "f+d+f-")
                           ("rdb" . "d-f+d+f-")
                           ("bdl" . "r-d-d-r+")
                           ("ldf" . "r-d+r+")
                           
                           ("drf" .     "r-d+r+f+d-d-f-")
                           ("dfl" .   "d+r-d+r+f+d-d-f-")
                           ("dlb" . "d-d-r-d+r+f+d-d-f-")
                           ("dbr" .   "d-r-d+r+f+d-d-f-")

                           ("fru" . "f+d-d-f-r-d-d-r+")
                           ("ruf" . "r-d-d-r+f+d-d-f-")
                           ("urb" . "b-d-d-b+r-d+r+")
                           ("rbu" . "r+d+r-r-d-d-r+")
                           ("bur" . "f+b-d-f-b+")
                           ("ubl" . "b+d-b-r-d-d-r+")
                           ("blu" . "b+r-d-d-r+b-")
                           ("lub" . "l-f+d-d-f-l+")
                           ("ulf" . "l+d-l-r-d+r+")
                           ("lfu" . "l+r-d+r+l-")
                           ("ful" . "f-d-f+f+d-d-f-"))))
         (if (not (null? moves))
             (if (and (eq? (lookup-colour (caar moves))
                           top-colour)
                      (eq? (lookup-colour (xsubstring (caar moves) 1 4))
                           front-colour)
                      (eq? (lookup-colour (xsubstring (caar moves) 2 5))
                           right-colour))
                 (do-moves (cdar moves))
                 (loop (cdr moves)))))))))

    


;; Consider the fr block. If the block is to be found in the bottom slice, get
;; it into one of two `starting positions' (either fd or rd) and then move it up
;; into fr by looping back. If the piece is not found here, it must be in one of
;; the other middle edge positions; for now we just check if we are holding a
;; middle edge block at fr and if so we drop it to the bottom slice, so that a
;; repeat of the algorithm will eventually put it in its correct place, and by
;; the time the repeat comes around our own block will hopefully have been
;; dropped down.

(define (mellor-middle-slice-solve)
  (do ((i 0 (+ 1 i))) ((eq? i 3))
    (repeat-for-all-rotations
     (lambda ()
       (let ((front-colour (lookup-colour "f"))
             (right-colour (lookup-colour "r")))
         (let loop ()
           (cond ((and (eq? (lookup-colour "fd") front-colour)
                       (eq? (lookup-colour "df") right-colour))
                  (do-moves "d-r-d+r+d+f+d-f-"))
                 ((and (eq? (lookup-colour "df") front-colour)
                       (eq? (lookup-colour "fd") right-colour))
                  (do-moves "d+") (loop))
                 ((and (eq? (lookup-colour "rd") right-colour)
                       (eq? (lookup-colour "dr") front-colour))
                  (do-moves "d+f+d-f-d-r-d+r+"))
                 ((and (eq? (lookup-colour "dr") right-colour)
                       (eq? (lookup-colour "rd") front-colour))
                  (do-moves "d-") (loop))
                 ((and (eq? (lookup-colour "ld") front-colour)
                       (eq? (lookup-colour "dl") right-colour))
                  (do-moves "d+") (loop))
                 ((and (eq? (lookup-colour "dl") front-colour)
                       (eq? (lookup-colour "ld") right-colour))
                  (do-moves "d+d+") (loop))
                 ((and (eq? (lookup-colour "bd") front-colour)
                       (eq? (lookup-colour "db") right-colour))
                  (do-moves "d+d+") (loop))
                 ((and (eq? (lookup-colour "db") front-colour)
                       (eq? (lookup-colour "bd") right-colour))
                  (do-moves "d-") (loop))

                 ((and (eq? (lookup-colour "fr") front-colour)
                       (eq? (lookup-colour "rf") right-colour))
                  noop)
                 
                 ((let ((bottom-colour (lookup-colour "d")))
                    (and (not (eq? (lookup-colour "fr") bottom-colour))
                         (not (eq? (lookup-colour "rf") bottom-colour))))
                  (do-moves "d+f+d-f-d-r-d+r+") (loop)))))))))




;; Procedure which answers the question, `Is the fdr block located in the right
;; place (regardless of orientation)?'

(define (placed-fdr?)
  (eq? (+ (ash 1 (lookup-colour "fdr"))
          (ash 1 (lookup-colour "dfr"))
          (ash 1 (lookup-colour "rdf")))
       (+ (ash 1 (lookup-colour "f"  ))
          (ash 1 (lookup-colour "d"  ))
          (ash 1 (lookup-colour "r"  )))))




;; We look at all the bottom corners, and set a bit in a bit-mask when one is in
;; the right place. There are six patterns to look for: four in which two
;; adjacent blocks are out of place, and two in which diagonal blocks are out of
;; place. When we find one of these, we apply a symbolic rotation to the cube so
;; that the missing pieces are in set positions, and then apply appriopriate set
;; moves. Otherwise, either all the pieces are already in place, or else we just
;; shift the bottom slice around and loop until we find a pattern.

(define (mellor-bottom-corner-place)
  (let ((fix-2 (lambda () (do-moves "r-d-r+f+d+f-r-d+r+d-d-")))
        (fix-d (lambda () (do-moves "r-d-r+f+d-d-f-r-d+r+d-"))))
    (let loop ()
      (case (do ((i 0 (+ i 1)) (a 0 a))
                ((eq? i 4) a)
              (rotated-flubrd-symbol-set! i)
              (set! a (+ a (if (placed-fdr?) (ash 1 i) 0))))
        ((3)  (rotated-flubrd-symbol-set! 3) (fix-2))
        ((6)  (rotated-flubrd-symbol-set! 0) (fix-2))
        ((12) (rotated-flubrd-symbol-set! 1) (fix-2))
        ((9)  (rotated-flubrd-symbol-set! 2) (fix-2))
        ((5)  (rotated-flubrd-symbol-set! 1) (fix-d))
        ((10) (rotated-flubrd-symbol-set! 0) (fix-d))
        ((15) noop)
        (else (do-moves "d+") (loop))))))




;; Very similar methodology to the above; look for patterns of blocks with the
;; right orientation, and then apply set-piece moves (depending on whether three
;; or two pieces are out of position). In all cases, we loop until we have a
;; solution because sometimes it takes more than one effort to get things
;; correct (we could go for a more intelligent approach than this...)

(define (mellor-bottom-corner-orient)
  (let ((base-colour (lookup-colour "d"))
        (fix-2 (lambda () (do-moves "f-f-r-d+r+f+d+f-u-f+d-f-r-d-r+u+f-f-")))
        (fix-3 (lambda () (do-moves "r-d-r+d-r-d-d-r+d-d-"))))
    (let loop ()
      (case (do ((i 0 (+ i 1)) (a 0 a))
                ((eq? i 4) a)
              (rotated-flubrd-symbol-set! i)
              (set! a (+ a (if (eq? (lookup-colour "dfr") base-colour)
                               (ash 1 i)
                               0))))
        ((0)  (rotated-flubrd-symbol-set! 0) (fix-3) (loop))
        ((8)  (rotated-flubrd-symbol-set! 0) (fix-3) (loop))
        ((1)  (rotated-flubrd-symbol-set! 1) (fix-3) (loop))
        ((2)  (rotated-flubrd-symbol-set! 2) (fix-3) (loop))
        ((4)  (rotated-flubrd-symbol-set! 3) (fix-3) (loop))
        ((6)  (rotated-flubrd-symbol-set! 0) (fix-2) (loop))
        ((12) (rotated-flubrd-symbol-set! 1) (fix-2) (loop))
        ((9)  (rotated-flubrd-symbol-set! 2) (fix-2) (loop))
        ((3)  (rotated-flubrd-symbol-set! 3) (fix-2) (loop))
        ((10 5) (fix-2) (loop))))))




;; Almost the same again; look at the pattern of correctly placed bottom edges,
;; and apply set-piece moves to a logically rotated cube so that the pieces are
;; in a certain pattern which we can solve. Again, it might take several efforts
;; to get this right, so we loop until a solution is found (again we could do
;; better than this if we tried!)

(define (mellor-bottom-edge-place)
  (let ((fix (lambda () (do-moves "r+l-f+r-l+d-d-r+l-f+r-l+"))))
    (let loop ()
      (case (do ((i 0 (+ 1 i)) (a 0 a))
                ((eq? i 4) a)
              (rotated-flubrd-symbol-set! i)
              (let ((side-colour (lookup-colour "f")))
                (set! a (+ a (if (or (eq? (lookup-colour "fd") side-colour)
                                     (eq? (lookup-colour "df") side-colour))
                                 (ash 1 i)
                                 0)))))
        ((0) (fix) (loop))
        ((1) (rotated-flubrd-symbol-set! 0) (fix) (loop))
        ((2) (rotated-flubrd-symbol-set! 1) (fix) (loop))
        ((4) (rotated-flubrd-symbol-set! 2) (fix) (loop))
        ((8) (rotated-flubrd-symbol-set! 3) (fix) (loop))))))




;; A similar approach again, but looking for patterns and applying moves to get
;; the bottom edges in the correct orientation.

(define (mellor-bottom-edge-orient)
  (let ((base-colour (get-colour-symbolic cube "d"))
        (fix-adjacent (lambda () (do-moves "f-f-r-r-r-u-d+b-b-u-u-d-d-f-u-f+u-u-d-d-b+b+u+d-r+u+r-r-f-f-")))
        (fix-opposite (lambda () (do-moves "r-r-l-l-r-u-d+b-b-u-u-d-d-f-u-u-f+u-u-d-d-b-b-u+d-r+u-u-r-r-l-l-"))))
    (let loop ()
      (case (do ((i 0 (+ i 1)) (a 0 a))
                ((eq? i 4) a)
              (rotated-flubrd-symbol-set! i)
              (set! a (+ a (if (eq? (lookup-colour "df") base-colour)
                               (ash 1 i)
                               0))))
        ((0) (fix-opposite) (loop))
        ((3) (rotated-flubrd-symbol-set! 2) (fix-adjacent))
        ((6) (rotated-flubrd-symbol-set! 3) (fix-adjacent))
        ((12) (rotated-flubrd-symbol-set! 0) (fix-adjacent))
        ((9) (rotated-flubrd-symbol-set! 1) (fix-adjacent))
        ((5) (rotated-flubrd-symbol-set! 0) (fix-opposite))
        ((10) (rotated-flubrd-symbol-set! 1) (fix-opposite))))))




;; Apply all the various stages for fixing a cube in order, but allow the user
;; to specify when to stop.

(define (mellor-solve stage)
  (set! cube (gnubik-cube-state))
  (if (check-cube-structure cube)
      (do ((i 0 (+ i 1)) (stage-list (list mellor-top-edge-solve
                                           mellor-top-corner-solve
                                           mellor-middle-slice-solve
                                           mellor-bottom-corner-place
                                           mellor-bottom-corner-orient
                                           mellor-bottom-edge-place
                                           mellor-bottom-edge-orient)
                                     (cdr stage-list)))
          ((eq? i stage) (execute-move-buffer!))
        ((car stage-list)))))




;; Provide plenty of menu entries to give the user some control over the solving
;; algorithm (he may want to perform part of the solution himself!)

(define _ gettext)

(define solvers (gnubik-create-menu (_ "_Solvers")))
(define m3 (gnubik-create-menu (_ "_3×3") solvers))
(define menu (gnubik-create-menu "_Mellor" m3))


(gnubik-register-script (_ "_Full cube")
                        '(mellor-solve 7) menu)

(gnubik-register-script (_ "Bottom _edge place")
                        '(mellor-solve 6) menu)

(gnubik-register-script (_ "Bottom _corner orient")
                        '(mellor-solve 5) menu)

(gnubik-register-script (_ "_Bottom corner place")
                        '(mellor-solve 4) menu)

(gnubik-register-script (_ "_Middle slice")
                        '(mellor-solve 3) menu)

(gnubik-register-script (_ "_Top slice")
                        '(mellor-solve 2) menu)

(gnubik-register-script (_ "_Top edges")
                        '(mellor-solve 1) menu)
