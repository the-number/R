;;; 20220810 (C) Gunter Liszewski, Emacs pov-mode package for Guix -*- mode: scheme; -*-

(define-module (gnu packages xen-0-emacs-abc)
  #:use-module ((guix licenses) #:prefix license:)
  #:use-module (guix packages)
  #:use-module (guix download)
  #:use-module (guix gexp)
  #:use-module (guix git-download)
  #:use-module (guix build-system copy)
  #:use-module (guix build-system emacs)
  #:use-module (gnu packages)
  #:use-module (gnu packages compression)
  #:use-module (gnu packages emacs)
  #:use-module (gnu packages texinfo)
  #:use-module (gnu packages xen-0-povray)
  #:use-module (guix utils)
  #:use-module (srfi srfi-1)
  #:use-module (ice-9 match))

;;;
;;; Emacs and POV-ray
;;;

(define-public emacs-pov-mode
  (package
    (name "emacs-pov-mode")
    (version "v3.3")
    (source
     (origin
       (method git-fetch)
       (uri (git-reference
             (url "https://github.com/emacsmirror/pov-mode")
             (commit version)))
       (file-name (git-file-name name version))
       (sha256
        (base32 "14silfng5rbdc8hnzswjmqk705pncjlk8iphjcxcm799h44pnlcr"))))
    (build-system emacs-build-system)
    (arguments
     (list
      #:phases
      #~(modify-phases %standard-phases
	 ;; Move the source files to the top level, which is included in
         ;; the EMACSLOADPATH.
         (add-after 'unpack 'move-source-files
           (lambda _
             (let ((el-files (find-files "./elisp" ".*\\.el$")))
               (for-each (lambda (f)
                           (rename-file f (basename f)))
                         el-files))))
	 (add-after 'move-source-files 'emacs-defcustom
	   (lambda* (#:key inputs #:allow-other-keys)
	     (make-file-writable "pov-mode.el")
	     (emacs-substitute-sexps "pov-mode.el"
	       ("defcustom pov-include-dir"
                 (string-append (assoc-ref inputs "povray-here")
				"/share/povray-3.7/include"))
	       ("defcustom  pov-documentation-directory"
	        (string-append (assoc-ref inputs "povray-here")
	     		"/share/doc/povray-3.7/html"))
	       ("defcustom pov-documentation-keyword-index"
				"r3_3.html")
	       ("defcustom pov-documentation-index"
				"index.html")
	       ("defcustom pov-insertmenu-location"
                 (string-append (assoc-ref inputs "povray-imenu-here")
			       "/share/povray-imenu-3.6/InsertMenu")))))
         (add-before 'install 'make-info
           (lambda _
             (with-directory-excursion "info"
               (invoke "makeinfo" "--no-split"
                       "-o" "pov-mode.info" "pov-mode.texi")))))))
    (native-inputs
     (list texinfo))
    (propagated-inputs
     (list povray-here povray-imenu-here))
    (home-page "https://github.com/emacsmirror/pov-mode")
    (synopsis "Major mode for editing POV-Ray scene files")
    (description
     "This major mode for GNU Emacs provides support for editing Povray
 scene files, rendering and viewing them.  It automatically indents
 blocks, both {} and #if #end.  It also provides context-sensitive
 keyword completion and font-lock highlighting, as well as the
 ability to look up those keywords in the povray documentation.")
    (license license:gpl3)))

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
