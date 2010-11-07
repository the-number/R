mkdir -p m4
aclocal -I m4 
autoconf
autoheader
build-aux/gitlog-to-changelog > ChangeLog
cat ChangeLog.OLD >> ChangeLog
automake --add-missing --copy
ls -1 src/*.c > po/POTFILES.in
