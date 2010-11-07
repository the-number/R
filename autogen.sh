mkdir -p build-aux
aclocal -I m4 
autoconf
autoheader
automake --add-missing --copy
ls -1 src/*.c > po/POTFILES.in
