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



;; For internationalisation
(define _ gettext)



;; Direct access to a cube object. It is assumed the user knows what he is doing
;; (read the manual!) :-)

(define (get-colour cube face index)
  (vector-ref (vector-ref (cdr cube) face)
              index))

(define (set-colour! cube face index colour)
  (vector-set! (vector-ref (cdr cube) face)
               index
               colour))




;; All the rest of this file assumes a certain structure to a cube object. This
;; test makes sure that structure is in place. The cube structure passed from
;; the C world starts with a list of structure version, number of dimensions,
;; and the size of the cube in each dimension.
;;
;; This function is not applied implicitly for efficiency's sake, but should be
;; exported from the module (when it becomes one) so that the user can check the
;; applicability of the module).

(define (check-cube-structure cube)
  (if (equal? (car cube) '(1 3 3 3 3))
      #t
      (begin (gnubik-error-dialog (_ "This script only works on 3×3×3 cubes."))
             #f)))




;; This a-list defines the relationships and orientations of the faces, as
;; passed from the C world into the scheme cube object. The key is the number of
;; the face, and then the values are the letter used to symbolically identify
;; the face, the face which runs along the 'top' edge of this face (the row
;; holding the 0, 1 and 2 cells), and the face which runs along the 'right' edge
;; of this face (the column with the 2, 5 and 8 cells).
;;
;; These correspondences have been determined by trial and error. There is also
;; an implicit assumption made that faces 0 and 1, 2 and 3, and faces 4 and 5
;; are opposites.

(define faces '((4 . (#\f 0 3))
                (5 . (#\b 0 3))
                (2 . (#\l 0 5))
                (3 . (#\r 0 5))
                (0 . (#\u 2 5))
                (1 . (#\d 2 5))))



;; Return the number of the face opposite the one given.

(define (opposite-face face)
  (+ (* 2 (quotient face 2)) (- 1 (modulo face 2))))



;; Alist with fbdurl strings to identify elements as the key, and a cons cell of
;; face and index as the value.

(define symbolic-mapping '())



;; A convenience to help construct the above alist.

(define (add-map symbol face index)
  (set! symbolic-mapping (cons (cons symbol
                                     (cons face index))
                               symbolic-mapping)))



;; For every face, make a mapping for each cell.

(for-each (lambda (face)


            ;; The centre cell.

            (add-map (string (cadr face)) (car face) 4)


            ;; The edge cells.

            (let ((add-map
                   (lambda (direction opposite index)
                     (add-map
                      (string
                       (cadr face)
                       (cadr (assv
                              ((if opposite opposite-face (lambda (x) x))
                               ((if direction cadddr caddr)
                                face))
                              faces)))
                      (car face)
                      index))))
              (add-map #f #f 1)
              (add-map #t #f 5)
              (add-map #f #t 7)
              (add-map #t #t 3))


            ;; The corner cells.

            (let* ((o (lambda (x) (if x opposite-face (lambda (x) x))))
                   (add-map (lambda (opp1 opp2 index)
                              (add-map
                               (string
                                (cadr face)
                                (cadr (assv ((o opp1) (caddr face)) faces))
                                (cadr (assv ((o opp2) (cadddr face)) faces)))
                               (car face)
                               index))))
              (add-map #f #f 2)
              (add-map #t #f 8)
              (add-map #f #t 0)
              (add-map #t #t 6))

            ;; The corner cells again (second and third components of the symbol
            ;; reversed).
            
            (let* ((o (lambda (x) (if x opposite-face (lambda (x) x))))
                   (add-map (lambda (opp1 opp2 index)
                              (add-map
                               (string
                                (cadr face)
                                (cadr (assv ((o opp2) (cadddr face)) faces))
                                (cadr (assv ((o opp1) (caddr face)) faces)))
                               (car face)
                               index))))
              (add-map #f #f 2)
              (add-map #t #f 8)
              (add-map #f #t 0)
              (add-map #t #t 6)))

          
          ;; The last clause of (for-each proc face).
          
          faces)




;; This is what it's all about. These are the two main functions which should be
;; exported from this module (when we make it a module).

(define (get-colour-symbolic cube symbol)
  (let ((symbol-entry (assoc symbol symbolic-mapping)))
    (get-colour cube (cadr symbol-entry) (cddr symbol-entry))))

(define (set-colour-symbolic! cube symbol colour)
  (let ((symbol-entry (assoc symbol symbolic-mapping)))
    (set-colour! cube (cadr symbol-entry) (cddr symbol-entry) colour)))



;;----------------------------------------------------------------------
;;
;; The second part of this file defines the functions needed to move the cube
;; about.
;;
;;----------------------------------------------------------------------



;; Create a procedure which rotates the letters of a string according to the
;; specification passed: l is a list of pairs of letters with the notion that
;; occurrences of the first should be replaced with the second, in the strings
;; that the returned procedure processes.

(define (define-face-twister sub-list)
  (lambda (s)
    (let ((s (string-copy s))
          (chars (string-length s)))
      (do ((i 0 (+ i 1))) ((eq? i chars) s)
        (let ((replacement (assoc-ref sub-list (string-ref s i))))
          (if replacement
              (string-set! s i replacement)))))))
      

  

;; In its natural state, this procedure manipulates the given scheme cube object
;; to reflect an anticlockwise rotation of the front face. However, the supplied
;; face-twister function may conspire to fiddle the face letters whenever the
;; cube is accessed so that, in face, any face may be the one being rotated.

(define (twist-front cube face-twister)
  (for-each
   
   (lambda (twists)
     (let ((first (get-colour-symbolic cube (face-twister (car twists)))))
       (let loop ((twists twists))
         (if (null? (cdr twists))
             (set-colour-symbolic! cube (face-twister (car twists)) first)
             (begin
               (set-colour-symbolic! cube
                                     (face-twister (car twists))
                                     (get-colour-symbolic cube
                                                          (face-twister
                                                           (cadr twists))))
               (loop (cdr twists)))))))
   
   '(("ufr" "rfd" "dfl" "lfu")
     ("uf" "rf" "df" "lf")
     ("ufl" "rfu" "dfr" "lfd")
     ("ful" "fur" "frd" "fld")
     ("fu" "fr" "fd" "fl"))))


  

;; Wrapper around the procedure above which works out a face-twister function
;; given the face the user wants rotating, and works out how many turns are
;; required given the user's notion of clockwise quarters.

(define (twist-face cube face quarters)
  (let ((face-twister
         (define-face-twister
           (case (string-ref face 0)
             ((#\f) '())
             ((#\b) '((#\b . #\f) (#\f . #\b) (#\l . #\r) (#\r . #\l)))
             ((#\u) '((#\u . #\b) (#\b . #\d) (#\d . #\f) (#\f . #\u)))
             ((#\d) '((#\d . #\b) (#\b . #\u) (#\u . #\f) (#\f . #\d)))
             ((#\l) '((#\l . #\b) (#\b . #\r) (#\r . #\f) (#\f . #\l)))
             ((#\r) '((#\r . #\b) (#\b . #\l) (#\l . #\f) (#\f . #\r)))))))
    (do ((i (case quarters ((1) 3) ((2) 2) ((3) 1) ((-1) 1))
            (- i 1)))
        ((eq? i 0))
      (twist-front cube face-twister))))




;; Implementation of the move buffer object. This list stores the moves
;; generated by the move-face procedure for eventual submission to the
;; gnubik-rotate-* procedures.

(define move-buffer '())
(define (execute-move-buffer!)
  (gnubik-append-moves (reverse! move-buffer))
  (set! move-buffer '()))
(define (clear-move-buffer!)
  (set! move-buffer '()))




;; A procedure which applies the above function for specific symbolic move
;; names. It also returns an entry that should be passed in a list to
;; gnubik-append-moves et al., to cause the cube to be rotated in the C world
;; also (the correspondence between the scheme moves and those in the C world
;; have been determined by trial and error).
;;
;; This is the third main function which should be exported from this module
;; (when it becomes one).

(define (move-face cube symbol)
  (let ((data (assoc symbol '(("f" .  (1 (0 -2 0)))
                              ("b" .  (1 (0  2 1)))
                              ("l" .  (1 (1 -2 0)))
                              ("r" .  (1 (1  2 1)))
                              ("u" .  (1 (2 -2 0)))
                              ("d" .  (1 (2  2 1)))
                              ("f+" . (1 (0 -2 0)))
                              ("b+" . (1 (0  2 1)))
                              ("l+" . (1 (1 -2 0)))
                              ("r+" . (1 (1  2 1)))
                              ("u+" . (1 (2 -2 0)))
                              ("d+" . (1 (2  2 1)))
                              ("f " . (1 (0 -2 0)))
                              ("b " . (1 (0  2 1)))
                              ("l " . (1 (1 -2 0)))
                              ("r " . (1 (1  2 1)))
                              ("u " . (1 (2 -2 0)))
                              ("d " . (1 (2  2 1)))
                              ("f-" . (3 (0 -2 1)))
                              ("b-" . (3 (0  2 0)))
                              ("l-" . (3 (1 -2 1)))
                              ("r-" . (3 (1  2 0)))
                              ("u-" . (3 (2 -2 1)))
                              ("d-" . (3 (2  2 0)))))))
    (twist-face cube symbol (cadr data))
    (set! move-buffer (cons (caddr data) move-buffer))))
