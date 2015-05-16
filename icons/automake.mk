themedir = $(DESTDIR)$(datadir)/icons/hicolor
context = apps
sizes = 16 22 24 32 48

install-icons:
	for size in $(sizes); do \
	  $(MKDIR_P) $(themedir)/$${size}x$${size}/$(context) ; \
          $(INSTALL_DATA) $(top_srcdir)/icons/logo$$size.png $(themedir)/$${size}x$${size}/$(context)/gnubik.png ; \
	done 
	if test -z $(DESTDIR) ; then \
	 gtk-update-icon-cache --ignore-theme-index $(themedir) ; \
	fi


uninstall-icons:
	for size in $(sizes); do \
          $(RM) $(themedir)/$${size}x$${size}/$(context)/gnubik.png ; \
	done 
	gtk-update-icon-cache --ignore-theme-index $(themedir)

desktopdir=$(DESTDIR)$(datadir)/applications

install-desktop:
	$(MKDIR_P) $(desktopdir)
	chmod u+w $(desktopdir)
	$(INSTALL_DATA) $(top_builddir)/icons/gnubik.desktop $(desktopdir)/gnubik.desktop

uninstall-desktop:
	$(RM) $(desktopdir)/gnubik.desktop


INSTALL_DATA_HOOKS = install-icons install-desktop

UNINSTALL_DATA_HOOKS = uninstall-icons uninstall-desktop

install-data-hook: $(INSTALL_DATA_HOOKS)
uninstall-hook: $(UNINSTALL_DATA_HOOKS)

EXTRA_DIST += icons/logo22.xcf icons/logo32.xcf icons/logo48.xcf
EXTRA_DIST += icons/logo16.png icons/logo22.png icons/logo32.png icons/logo48.png
EXTRA_DIST += icons/gen-dot-desktop.scm


icons/gnubik.desktop: icons/gen-dot-desktop.scm  $(POFILES)
	$(MKDIR_P) `dirname $@`
	$(GUILE) $< `echo $(POFILES) | sed -e 's%po/%'$(srcdir)'/po/%g'` > $@,tmp
	mv $@,tmp $@


ALL_LOCAL += icons/gnubik.desktop

CLEANFILES += icons/gnubik.desktop
