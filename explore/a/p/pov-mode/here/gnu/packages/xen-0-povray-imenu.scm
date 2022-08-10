;;; 20220810 (C) Gunter Liszewski, Emacs pov-mode package for Guix -*- mode: scheme; -*-

(define-module (gnu packages xen-0-povray-imenu)
  #:use-module ((guix licenses) #:prefix license:)
  #:use-module (guix packages)
  #:use-module (guix download)
;;  #:use-module (guix git-download)
;;  #:use-module (guix build-system gnu)
  #:use-module (guix build-system copy)
  #:use-module (gnu packages)
  #:use-module (gnu packages compression))

(define-public povray-imenu-here
  (package
    (name "povray-imenu-here")
    (version "3.6")
    (source
     (origin
      (method url-fetch)
      (uri
       (string-append
	"http://xahlee.info/3d/i/povray-imenu-" version ".zip"))
      (sha256
       (base32
        "0j79xrs4cs5awyi5zdq7kil7i9wr95blasnj1kqbzac9b62rhdsx"))))
    (build-system copy-build-system)
    (arguments
     '(#:install-plan
       '(("." "share/povray-imenu-3.6" #:exclude ("COPYING")))))
    (native-inputs (list unzip))
    (synopsis "POV-ray templates for emacs-pov-mode")
    (description
     "The content of this package has been created from the 
POV-Ray 3.6 insert menu that is included with the Windows 
and Mac versions of POV-Ray.

This package is provided to simplify access to the insert 
menu without the need to obtain and extract the Windows 
binary package.  It is covered by the POV-Ray license and
may be used by anyone eligible to use POV-Ray according to 
this license.")
    (home-page "http://www.imagico.de/imenu")
    (license license:agpl3+)))
