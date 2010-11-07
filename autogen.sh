if [ -z "$AC_ARCHIVE_DIR" ] ; then
    printf "You must set the variable AC_ARCHIVE_DIR to the location of the autoconf archive\n"
    exit 1;
fi;
mkdir -p m4
for x in  ax_check_gl.m4 ax_check_glu.m4 ax_check_glut.m4 ax_lang_compiler_ms.m4 ax_pthread.m4 ; do
  cp $AC_ARCHIVE_DIR/m4/$x m4
done
aclocal -I m4 
autoconf
autoheader
build-aux/gitlog-to-changelog > ChangeLog
cat ChangeLog.OLD >> ChangeLog
automake --add-missing --copy

