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
	src/cube.c src/cube.h \
	src/cube_i.h \
	src/cubeview.c src/cubeview.h \
	src/colour-dialog.c src/colour-dialog.h \
	src/control.h src/control.c \
	src/cursors.c src/cursors.h \
	src/dialogs.c src/dialogs.h \
	src/drwBlock.c src/drwBlock.h \
	src/game.c src/game.h \
	src/glarea-common.c \
	src/glarea.h \
	src/main.c \
	src/move.c src/move.h \
	src/quarternion.c src/quarternion.h \
	src/select.c src/select.h \
	src/txfm.c src/txfm.h \
	src/textures.c src/textures.h \
	src/ui.h \
	src/version.c src/version.h \
	src/widget-set.c src/widget-set.h 


