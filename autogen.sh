if [ -z "$AC_ARCHIVE_DIR" ] ; then
    printf "You must set the variable AC_ARCHIVE_DIR to the location of the autoconf archive\n"
    exit 1;
fi;

make -f autogen.mak AC_ARCHIVE_DIR=$AC_ARCHIVE_DIR
