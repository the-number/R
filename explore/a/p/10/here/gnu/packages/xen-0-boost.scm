;;; 20220820 (C) Gunter Liszewski -- see (gnu packages boost)

(define-module (gnu packages xen-0-boost)
  #:use-module ((guix licenses) #:prefix license:)
  #:use-module (guix utils)
  #:use-module (guix packages)
  #:use-module (guix download)
  #:use-module (guix git-download)
  #:use-module (guix build-system gnu)
  #:use-module (guix build-system trivial)
  #:use-module (gnu packages)
  #:use-module (gnu packages compression)
  #:use-module (gnu packages hurd)
  #:use-module (gnu packages icu4c)
  #:use-module (gnu packages llvm)
  #:use-module (gnu packages perl)
  #:use-module (gnu packages python)
  #:use-module (gnu packages shells)
  #:use-module (gnu packages mpi)
  #:use-module (srfi srfi-1))

(define (version-with-underscores version)
  (string-map (lambda (x) (if (eq? x #\.) #\_ x)) version))

(define (boost-patch name version hash)
  (origin
    (method url-fetch)
    (uri (string-append "https://www.boost.org/patches/"
                        (version-with-underscores version) "/" name))
    (file-name (string-append "boost-" name))
    (sha256 (base32 hash))))

(define-public boost-here
  (package
    (name "boost-here")
    (version "1.79.0")
    (source (origin
              (method url-fetch)
              (uri (string-append "https://boostorg.jfrog.io/artifactory/main/release/"
                                  version "/source/boost_"
                                  (version-with-underscores version) ".tar.bz2"))
              (sha256
               (base32
		"0fggarccddj6q4ifj3kn7g565rbhn4ia1vd45fxb7y57a6fmhpa7"))))
    (build-system gnu-build-system)
    (inputs (list icu4c zlib))
    (native-inputs
     `(("perl" ,perl)
       ,@(if (%current-target-system)
             '()
             `(("python" ,python-minimal-wrapper)))
       ("tcsh" ,tcsh)))
    (arguments
     `(#:imported-modules ((guix build python-build-system)
                           ,@%gnu-build-system-modules)
       #:modules (((guix build python-build-system) #:select (python-version))
                  ,@%gnu-build-system-modules)
       #:tests? #f
       #:make-flags
       (list "threading=multi" "link=shared"

             ;; Set the RUNPATH to $libdir so that the libs find each other.
             (string-append "linkflags=-Wl,-rpath="
                            (assoc-ref %outputs "out") "/lib")
             ,@(if (%current-target-system)
                   `("--user-config=user-config.jam"
                     ;; Python is not supported when cross-compiling.
                     "--without-python"
                     "binary-format=elf"
                     "target-os=linux"
                     ,@(cond
                        ((string-prefix? "arm" (%current-target-system))
                         '("abi=aapcs"
                           "address-model=32"
                           "architecture=arm"))
                        ((string-prefix? "aarch64" (%current-target-system))
                         '("abi=aapcs"
                           "address-model=64"
                           "architecture=arm"))
                        (else '())))
                   '()))
       #:phases
       (modify-phases %standard-phases
         (delete 'bootstrap)
         (replace 'configure
           (lambda* (#:key inputs outputs #:allow-other-keys)
             (let ((icu (assoc-ref inputs "icu4c"))
                   (python (assoc-ref inputs "python"))
                   (out (assoc-ref outputs "out")))
               (substitute* '("libs/config/configure"
                              "libs/spirit/classic/phoenix/test/runtest.sh"
                              "tools/build/src/engine/execunix.cpp")
                 (("/bin/sh") (which "sh")))

               (setenv "SHELL" (which "sh"))
               (setenv "CONFIG_SHELL" (which "sh"))

               ,@(if (%current-target-system)
                     `((call-with-output-file "user-config.jam"
                          (lambda (port)
                            (format port
                                    "using gcc : cross : ~a-c++ ;"
                                    ,(%current-target-system)))))
                     '())

               ;; Change an #ifdef __MACH__ that really targets macOS.
               (substitute* "boost/test/utils/timer.hpp"
                 (("defined\\(__MACH__\\)")
                  "(defined __MACH__ && !defined __GNU__)"))

               (invoke "./bootstrap.sh"
                       (string-append "--prefix=" out)
                       ;; Auto-detection looks for ICU only in traditional
                       ;; install locations.
                       (string-append "--with-icu=" icu)
                       ;; Ditto for Python.
                       ,@(if (%current-target-system)
                             '()
                             `((string-append "--with-python-root=" python)
                               (string-append "--with-python=" python "/bin/python")
                               (string-append "--with-python-version="
                                              (python-version python))))
                       "--with-toolset=gcc"))))
         (replace 'build
           (lambda* (#:key make-flags #:allow-other-keys)
             (apply invoke "./b2"
                    (format #f "-j~a" (parallel-job-count))
                    make-flags)))
         (replace 'install
           (lambda* (#:key make-flags #:allow-other-keys)
             (apply invoke "./b2" "install" make-flags)))
         ,@(if (%current-target-system)
               '()
               '((add-after 'install 'provide-libboost_python
                    (lambda* (#:key inputs outputs #:allow-other-keys)
                      (let* ((out (assoc-ref outputs "out"))
                             (python-version (python-version
                                              (assoc-ref inputs "python")))
                             (libboost_pythonNN.so
                              (string-append "libboost_python"
                                             (string-join (string-split
                                                           python-version #\.)
                                                          "")
                                             ".so")))
                        (with-directory-excursion (string-append out "/lib")
                          (symlink libboost_pythonNN.so "libboost_python.so")
                          ;; Some packages only look for the major version.
                          (symlink libboost_pythonNN.so
                                   (string-append "libboost_python"
                                                  (string-take python-version 1)
                                                  ".so")))))))))))

    (home-page "https://www.boost.org")
    (synopsis "Peer-reviewed portable C++ source libraries")
    (description
     "A collection of libraries intended to be widely useful, and usable
across a broad spectrum of applications.")
    (license (license:x11-style "https://www.boost.org/LICENSE_1_0.txt"
                                "Some components have other similar licences."))))

(define-public boost-here-1.80.0
  (package
    (name "boost-here")
    (version "1.80.0")
    (source (origin
              (method url-fetch)
              (uri (string-append "https://boostorg.jfrog.io/artifactory/main/release/"
                                  version "/source/boost_"
                                  (version-with-underscores version) ".tar.bz2"))
              (sha256
               (base32
		"0fggarccddj6q4ifj3kn7g565rbhn4ia1vd45fxb7y57a6fmhpa7"))))
    (build-system gnu-build-system)
    (inputs (list icu4c zlib))
    (native-inputs
     `(("perl" ,perl)
       ,@(if (%current-target-system)
             '()
             `(("python" ,python-minimal-wrapper)))
       ("tcsh" ,tcsh)))
    (arguments
     `(#:imported-modules ((guix build python-build-system)
                           ,@%gnu-build-system-modules)
       #:modules (((guix build python-build-system) #:select (python-version))
                  ,@%gnu-build-system-modules)
       #:tests? #f
       #:make-flags
       (list "threading=multi" "link=shared"

             ;; Set the RUNPATH to $libdir so that the libs find each other.
             (string-append "linkflags=-Wl,-rpath="
                            (assoc-ref %outputs "out") "/lib")
             ,@(if (%current-target-system)
                   `("--user-config=user-config.jam"
                     ;; Python is not supported when cross-compiling.
                     "--without-python"
                     "binary-format=elf"
                     "target-os=linux"
                     ,@(cond
                        ((string-prefix? "arm" (%current-target-system))
                         '("abi=aapcs"
                           "address-model=32"
                           "architecture=arm"))
                        ((string-prefix? "aarch64" (%current-target-system))
                         '("abi=aapcs"
                           "address-model=64"
                           "architecture=arm"))
                        (else '())))
                   '()))
       #:phases
       (modify-phases %standard-phases
         (delete 'bootstrap)
         (replace 'configure
           (lambda* (#:key inputs outputs #:allow-other-keys)
             (let ((icu (assoc-ref inputs "icu4c"))
                   (python (assoc-ref inputs "python"))
                   (out (assoc-ref outputs "out")))
               (substitute* '("libs/config/configure"
                              "libs/spirit/classic/phoenix/test/runtest.sh"
                              "tools/build/src/engine/execunix.cpp")
                 (("/bin/sh") (which "sh")))

               (setenv "SHELL" (which "sh"))
               (setenv "CONFIG_SHELL" (which "sh"))

               ,@(if (%current-target-system)
                     `((call-with-output-file "user-config.jam"
                          (lambda (port)
                            (format port
                                    "using gcc : cross : ~a-c++ ;"
                                    ,(%current-target-system)))))
                     '())

               ;; Change an #ifdef __MACH__ that really targets macOS.
               (substitute* "boost/test/utils/timer.hpp"
                 (("defined\\(__MACH__\\)")
                  "(defined __MACH__ && !defined __GNU__)"))

               (invoke "./bootstrap.sh"
                       (string-append "--prefix=" out)
                       ;; Auto-detection looks for ICU only in traditional
                       ;; install locations.
                       (string-append "--with-icu=" icu)
                       ;; Ditto for Python.
                       ,@(if (%current-target-system)
                             '()
                             `((string-append "--with-python-root=" python)
                               (string-append "--with-python=" python "/bin/python")
                               (string-append "--with-python-version="
                                              (python-version python))))
                       "--with-toolset=gcc"))))
         (replace 'build
           (lambda* (#:key make-flags #:allow-other-keys)
             (apply invoke "./b2"
                    (format #f "-j~a" (parallel-job-count))
                    make-flags)))
         (replace 'install
           (lambda* (#:key make-flags #:allow-other-keys)
             (apply invoke "./b2" "install" make-flags)))
         ,@(if (%current-target-system)
               '()
               '((add-after 'install 'provide-libboost_python
                    (lambda* (#:key inputs outputs #:allow-other-keys)
                      (let* ((out (assoc-ref outputs "out"))
                             (python-version (python-version
                                              (assoc-ref inputs "python")))
                             (libboost_pythonNN.so
                              (string-append "libboost_python"
                                             (string-join (string-split
                                                           python-version #\.)
                                                          "")
                                             ".so")))
                        (with-directory-excursion (string-append out "/lib")
                          (symlink libboost_pythonNN.so "libboost_python.so")
                          ;; Some packages only look for the major version.
                          (symlink libboost_pythonNN.so
                                   (string-append "libboost_python"
                                                  (string-take python-version 1)
                                                  ".so")))))))))))

    (home-page "https://www.boost.org")
    (synopsis "Peer-reviewed portable C++ source libraries")
    (description
     "A collection of libraries intended to be widely useful, and usable
across a broad spectrum of applications.")
    (license (license:x11-style "https://www.boost.org/LICENSE_1_0.txt"
                                "Some components have other similar licences."))))


(define-public b2-here-1.80.0
  (package
    (name "b2-here")
    (version "boost-1.80.0")
    (source (origin
             (method git-fetch)
	     (uri
	      (git-reference
	       (url
		"https://github.com/boostorg/build.git")
	       (commit version)))
	     (file-name (git-file-name name version))
             (sha256
              (base32
	       "06l1rbj0shal6v6y1zszwhzb7f7lbfx4k13lsjvyc6559l366srw"))))
    (build-system gnu-build-system)
    (inputs (list icu4c zlib))
    (native-inputs
     `(("perl" ,perl)
       ,@(if (%current-target-system)
             '()
             `(("python" ,python-minimal-wrapper)))
       ("tcsh" ,tcsh)))
    (arguments
     `(
       ;; #:imported-modules ((guix build python-build-system)
       ;;                     ,@%gnu-build-system-modules)
       ;; #:modules (((guix build python-build-system) #:select (python-version))
       ;;            ,@%gnu-build-system-modules)
       #:tests? #f
       ;; #:make-flags
       ;; (list "threading=multi" "link=shared"

       ;;       ;; Set the RUNPATH to $libdir so that the libs find each other.
       ;;       (string-append "linkflags=-Wl,-rpath="
       ;;                      (assoc-ref %outputs "out") "/lib")
       ;;       ,@(if (%current-target-system)
       ;;             `("--user-config=user-config.jam"
       ;;               ;; Python is not supported when cross-compiling.
       ;;               "--without-python"
       ;;               "binary-format=elf"
       ;;               "target-os=linux"
       ;;               ,@(cond
       ;;                  ((string-prefix? "arm" (%current-target-system))
       ;;                   '("abi=aapcs"
       ;;                     "address-model=32"
       ;;                     "architecture=arm"))
       ;;                  ((string-prefix? "aarch64" (%current-target-system))
       ;;                   '("abi=aapcs"
       ;;                     "address-model=64"
       ;;                     "architecture=arm"))
       ;;                  (else '())))
       ;;             '()))
       #:phases
       (modify-phases %standard-phases
         (delete 'bootstrap)
         (replace 'configure
           (lambda* (#:key inputs outputs #:allow-other-keys)
             (let ((icu (assoc-ref inputs "icu4c"))
                   (python (assoc-ref inputs "python"))
                   (out (assoc-ref outputs "out")))
               (substitute* '("src/engine/execunix.cpp")
                 (("/bin/sh") (which "sh")))

               (setenv "SHELL" (which "sh"))
               (setenv "CONFIG_SHELL" (which "sh"))

               ,@(if (%current-target-system)
                     `((call-with-output-file "user-config.jam"
                          (lambda (port)
                            (format port
                                    "using gcc : cross : ~a-c++ ;"
                                    ,(%current-target-system)))))
                     '())

               ;; ;; Change an #ifdef __MACH__ that really targets macOS.
               ;; (substitute* "boost/test/utils/timer.hpp"
               ;;   (("defined\\(__MACH__\\)")
               ;;    "(defined __MACH__ && !defined __GNU__)"))

               (invoke "./bootstrap.sh"
                       (string-append "--prefix=" out)
                       ;; Auto-detection looks for ICU only in traditional
                       ;; install locations.
                       (string-append "--with-icu=" icu)
                       ;; Ditto for Python.
                       ,@(if (%current-target-system)
                             '()
                             `((string-append "--with-python-root=" python)
                               (string-append "--with-python=" python "/bin/python")
                               ;; (string-append "--with-python-version="
                               ;;                (python-version python))
			       ))
                       "--with-toolset=gcc"))))
         (replace 'build
           (lambda* (#:key make-flags #:allow-other-keys)
             (apply invoke "./b2"
                    (format #f "-j~a" (parallel-job-count))
                    make-flags)))
         (replace 'install
	   (lambda* (#:key make-flags outputs #:allow-other-keys)
	     (apply invoke "./b2" "install"
	       (string-append "--prefix=" (assoc-ref outputs "out"))
		  make-flags)))
         ;; ,@(if (%current-target-system)
         ;;       '()
         ;;       '((add-after 'install 'provide-libboost_python
         ;;            (lambda* (#:key inputs outputs #:allow-other-keys)
         ;;              (let* ((out (assoc-ref outputs "out"))
         ;;                     (python-version (python-version
         ;;                                      (assoc-ref inputs "python")))
         ;;                     (libboost_pythonNN.so
         ;;                      (string-append "libboost_python"
         ;;                                     (string-join (string-split
         ;;                                                   python-version #\.)
         ;;                                                  "")
         ;;                                     ".so")))
         ;;                (with-directory-excursion (string-append out "/lib")
         ;;                  (symlink libboost_pythonNN.so "libboost_python.so")
         ;;                  ;; Some packages only look for the major version.
         ;;                  (symlink libboost_pythonNN.so
         ;;                           (string-append "libboost_python"
         ;;                                          (string-take python-version 1)
         ;;                                          ".so"))))))))
	 )))

    (home-page "https://www.boost.org")
    (synopsis "b2: TODO: Peer-reviewed portable C++ source libraries")
    (description
     "A collection of libraries intended to be widely useful, and usable
across a broad spectrum of applications.")
    (license (license:x11-style "https://www.boost.org/LICENSE_1_0.txt"
                                "Some components have other similar licences."))))


