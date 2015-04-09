;; -*-scheme-*-
(use-modules (ice-9 popen))
(use-modules (ice-9 rdelim))

(define (N_ x) x)

(define generic-name (N_ "GNUbik"))
(define comment (N_ "A 3D magic cube puzzle"))

(define (thing p)
  (let ((str (read-line p)))
    (if (not (eof-object? str))
	(if (and (> (string-length str) 9) 
		 (string-contains str "msgstr \"" 0 9))
	    (string-drop-right (string-drop str 8) 1)
	    (thing p))
	#f)))

(define locs (cdr (program-arguments)))

(define (grep-generic-name what file)
  (let ((p (open-input-pipe
	    (format #f
		    "msggrep -K -e '^~A$' '~A' 2> /dev/null | msgattrib --translated --no-fuzzy" what file)))
	)
    (let ((x (thing p)))

    (close p)
    x)))

;; Convert po/de.po to de
(define (filename->isocode f)
  (string-drop-right (string-drop f 3) 3))

(define (N_ x) x)

(display "# Generated file. Do not edit. -*- buffer-read-only: t -*-\n\n")

(display "[Desktop Entry]\n")
(display "Name=Gnubik\n")
(format #t "GenericName=~A\n" (N_ generic-name))
(format #t "Comment=~A\n" (N_ "A 3D magic cube puzzle"))

(map 
 (lambda (loc)
   (let ((str  (grep-generic-name generic-name (format #f "~A" loc))))
     (if str
	 (display (format #f "GenericName[~A]=~A\n" (filename->isocode loc) str))))) locs)

(map 
 (lambda (loc)
   (let ((str  (grep-generic-name comment (format #f "~A" loc))))
     (if str
	 (display (format #f "Comment[~A]=~A\n" (filename->isocode loc) str))))) locs)

(display "Exec=gnubik %F\n")
(display "Icon=gnubik\n")
(display "Terminal=false\n")
(display "Type=Application\n")
(display "Categories=Game;Amusement;GTK;\n")

