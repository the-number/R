M4=m4/ax_check_gl.m4 m4/ax_check_glu.m4 m4/ax_check_glut.m4 m4/ax_lang_compiler_ms.m4 m4/ax_pthread.m4

AM_SFX=$(if $(AM_VERS),-$(AM_VERS),)

ACLOCAL=aclocal$(AM_SFX)
AUTOMAKE=automake$(AM_SFX)

all: Makefile.in configure

configure: aclocal.m4
	autoconf

aclocal.m4: $(M4)
	$(ACLOCAL) -I m4

config.h.in: configure.ac
	autoheader

BUILDAUX_DIR=build-aux

AUXFILES=$(BUILDAUX_DIR)/compile $(BUILDAUX_DIR)/config.guess $(BUILDAUX_DIR)/config.sub $(BUILDAUX_DIR)/install-sh $(BUILDAUX_DIR)/missing $(BUILDAUX_DIR)/depcomp $(BUILDAUX_DIR)/texinfo.tex

Makefile.in: aclocal.m4 config.h.in ChangeLog
	$(AUTOMAKE) --add-missing --copy

.PHONY:ChangeLog
ChangeLog: 
	$(BUILDAUX_DIR)/gitlog-to-changelog > $@
	cat ChangeLog.OLD >> $@


.PHONY: clean
clean:
	$(RM) -r m4
	$(RM) configure aclocal.m4
	$(RM) config.h.in
	$(RM) Makefile.in $(AUXFILES)





