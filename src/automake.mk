bin_PROGRAMS = src/gnubik


AM_CPPFLAGS+=-DGUILEDIR=\"$(pkgdatadir)\" \
    -DSCRIPTDIR=\"$(pkgdatadir)/scripts\" 


src_gnubik_CPPFLAGS = $(AM_CPPFLAGS)

src_gnubik_CFLAGS = $(AM_CFLAGS) \
	$(GTK_CFLAGS) \
	$(GL_CFLAGS) \
	$(GLU_CFLAGS) \
	$(GLUT_CFLAGS) \
	$(GDK_GL_EXT_CFLAGS) \
	$(GTK_GL_EXT_CFLAGS) \
	-DGDK_MULTIHEAD_SAFE=1

src_gnubik_LDADD = $(GTK_LIBS) $(GDK_GL_EXT_LIBS) $(GTK_GL_EXT_LIBS) \
	 $(GL_LIBS) \
	 $(GLU_LIBS) \
	 $(GLUT_LIBS)


src_gnubik_SOURCES = \
	src/colour-sel.c  \
	src/colour-sel.h \
	src/cube.c \
	src/cube.h \
	src/cube_i.h \
	src/cursors.c  \
	src/cursors.h \
	src/drwBlock.c \
	src/drwBlock.h \
	src/glarea-common.c \
	src/glarea.c    \
	src/glarea.h \
	src/gnubik.h \
	src/gnubik.xpm \
	src/guile-hooks.c  \
	src/guile-hooks.h  \
	src/main.c \
	src/menus.c \
	src/menus-gtk.h \
	src/move-queue.c \
	src/move-queue.h \
	src/move-queue_i.h \
	src/quarternion.c  \
	src/quarternion.h \
	src/select.c    \
	src/select.h \
	src/textures.c  \
	src/textures.h \
	src/txfm.c \
	src/txfm.h \
	src/ui.c \
	src/ui.h \
	src/version.c \
	src/version.h \
	src/widget-set.c \
	src/widget-set.h 


po/POTFILES.in:
	echo $(src_gnubik_SOURCES) | sed -e 's/ /\
/g' > $@

