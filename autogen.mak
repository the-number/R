M4=m4/ax_check_gl.m4 m4/ax_check_glu.m4 m4/ax_check_glut.m4 m4/ax_lang_compiler_ms.m4 m4/ax_pthread.m4

all: Makefile.in configure

configure: aclocal.m4
	autoconf

aclocal.m4: $(M4)
	aclocal -I m4

config.h.in: configure.ac
	autoheader

m4/%.m4: $(AC_ARCHIVE_DIR)/m4/%.m4
	@mkdir -p m4
	cp $< $@

BUILDAUX_DIR=build-aux

AUXFILES=$(BUILDAUX_DIR)/compile $(BUILDAUX_DIR)/config.guess $(BUILDAUX_DIR)/config.sub $(BUILDAUX_DIR)/install-sh $(BUILDAUX_DIR)/missing $(BUILDAUX_DIR)/depcomp $(BUILDAUX_DIR)/texinfo.tex

Makefile.in: aclocal.m4 config.h.in ChangeLog
	automake --add-missing --copy

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





