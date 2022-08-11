;;; 20220519 (C) Gunter Liszewski 

(define-module (gnu packages xen-0-povray)
  #:use-module (gnu packages)
  #:use-module (gnu packages algebra)
  #:use-module (gnu packages audio)
  #:use-module (gnu packages autotools)
  #:use-module (gnu packages base)
  #:use-module (gnu packages bash)
  #:use-module (gnu packages bison)
  #:use-module (gnu packages boost)
  #:use-module (gnu packages xen-0-boost)
  #:use-module (gnu packages build-tools)
  #:use-module (gnu packages cdrom)
  #:use-module (gnu packages check)
  #:use-module (gnu packages compression)
  #:use-module (gnu packages crypto)
  #:use-module (gnu packages datastructures)
  #:use-module (gnu packages documentation)
  #:use-module (gnu packages flex)
  #:use-module (gnu packages fonts)
  #:use-module (gnu packages fontutils)
  #:use-module (gnu packages freedesktop)
  #:use-module (gnu packages gettext)
  #:use-module (gnu packages ghostscript)
  #:use-module (gnu packages gl)
  #:use-module (gnu packages glib)
  #:use-module (gnu packages gnome)
  #:use-module (gnu packages gnunet)
  #:use-module (gnu packages graphics)
  #:use-module (gnu packages graphviz)
  #:use-module (gnu packages gstreamer)
  #:use-module (gnu packages gtk)
  #:use-module (gnu packages haskell-xyz)
  #:use-module (gnu packages image)
  #:use-module (gnu packages image-processing)
  #:use-module (gnu packages imagemagick)
  #:use-module (gnu packages jemalloc)
  #:use-module (gnu packages kde-frameworks)
  #:use-module (gnu packages libusb)
  #:use-module (gnu packages linux)
  #:use-module (gnu packages logging)
  #:use-module (gnu packages llvm)
  #:use-module (gnu packages lua)
  #:use-module (gnu packages maths)
  #:use-module (gnu packages mp3)
  #:use-module (gnu packages multiprecision)
  #:use-module (gnu packages ninja)
  #:use-module (gnu packages pciutils)
  #:use-module (gnu packages pdf)
  #:use-module (gnu packages perl)
  #:use-module (gnu packages photo)
  #:use-module (gnu packages pkg-config)
  #:use-module (gnu packages plotutils)
  #:use-module (gnu packages pretty-print)
  #:use-module (gnu packages pth)
  #:use-module (gnu packages pulseaudio)  ; libsndfile, libsamplerate
  #:use-module (gnu packages python)
  #:use-module (gnu packages python-build)
  #:use-module (gnu packages python-xyz)
  #:use-module (gnu packages qt)
  #:use-module (gnu packages readline)
  #:use-module (gnu packages sdl)
  #:use-module (gnu packages serialization)
  #:use-module (gnu packages stb)
  #:use-module (gnu packages swig)
  #:use-module (gnu packages tbb)
  #:use-module (gnu packages toolkits)
  #:use-module (gnu packages upnp)
  #:use-module (gnu packages video)
  #:use-module (gnu packages vulkan)
  #:use-module (gnu packages xiph)
  #:use-module (gnu packages xml)
  #:use-module (gnu packages xorg)
  #:use-module (gnu packages xdisorg)
  #:use-module (guix build-system copy)
  #:use-module (guix build-system cmake)
  #:use-module (guix build-system gnu)
  #:use-module (guix build-system meson)
  #:use-module (guix build-system python)
  #:use-module (guix build-system qt)
  #:use-module (guix download)
  #:use-module (guix gexp)
  #:use-module (guix git-download)
  #:use-module (guix hg-download)
  #:use-module ((guix licenses) #:prefix license:)
  #:use-module (guix packages)
  #:use-module (guix utils))

(define-public povray-here
  (package
    (name "povray-here")
    (version "3.7.0.10")
    (source (origin
              (method git-fetch)
              (uri (git-reference
                    (url "https://github.com/POV-Ray/povray")
                    (commit (string-append "v" version))))
              (file-name (git-file-name name version))
              (sha256
               (base32
		"19bv962clwc6sk53kq8bqf77fh0v46afm2knjbki8yj0m1mnyyd0"))
              (modules '((guix build utils)))
              (snippet
               '(begin
                  ;; Delete bundled libraries.
                  (delete-file-recursively "libraries")
                  #t))))
    (build-system gnu-build-system)
    (native-inputs
     (list autoconf automake pkg-config))
    (inputs
     `(("boost" ,boost-here) ;; XXX: this is 1.79
       ;;; ("boost" ,boost-for-irods) ;; XXX/TODO: use boost 1.79 to fix the _Pragma'tics
       ("libjpeg" ,libjpeg-turbo)
       ("libpng" ,libpng)
       ("libtiff" ,libtiff)
       ("openexr" ,openexr-2)
       ("sdl" ,sdl)
       ("zlib" ,zlib)))
    (arguments
     '(#:configure-flags
       (list "COMPILED_BY=Guix-S10"
	     ;;; "CPPFLAGS= -DBOOST_BIND_GLOBAL_PLACEHOLDERS -DBOOST_MATH_TOOLS_HEADER_DEPRECATED"
	     ;;; "CXXFLAGS=-std=c++11 -DBOOST_BIND_GLOBAL_PLACEHOLDERS -no-integrated-cpp"
	     "CXXFLAGS=-std=c++11 -DBOOST_BIND_GLOBAL_PLACEHOLDERS"
	     ;;; "--without-x"
             (string-append "--with-boost-libdir="
                            (assoc-ref %build-inputs "boost") "/lib")
             "--disable-optimiz-arch")
       #:phases
       (modify-phases %standard-phases
         (add-after 'unpack 'run-prebuild
           (lambda _
             (setenv "HOME" (getcwd))
	     ;;; (setenv "BOOST_BIND_GLOBAL_PLACEHOLDERS" "1")
             (with-directory-excursion "unix"
               (substitute* "prebuild.sh"
                 (("/bin/sh") (which "sh")))
               (invoke "sh" "prebuild.sh"))
             #t))
         ;; The bootstrap script is run by the prebuild script in the
         ;; "run-prebuild" phase.
         (delete 'bootstrap)
	 #;(delete 'check))))
    (synopsis "Tool for creating three-dimensional graphics")
    (description
     "@code{POV-Ray} is short for the Persistence of Vision Raytracer, a tool
for producing high-quality computer graphics.  @code{POV-Ray} creates
three-dimensional, photo-realistic images using a rendering technique called
ray-tracing.  It reads in a text file containing information describing the
objects and lighting in a scene and generates an image of that scene from the
view point of a camera also described in the text file.  Ray-tracing is not a
fast process by any means, but it produces very high quality images with
realistic reflections, shading, perspective and other effects.")
    (home-page "http://www.povray.org/")
    (license license:agpl3+)))

