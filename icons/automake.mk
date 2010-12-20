themedir = $(DESTDIR)$(datadir)/icons/hicolor
context = apps
sizes = 16 22 24 32 48

install-icons:
	for size in $(sizes); do \
	  $(MKDIR_P) $(themedir)/$${size}x$${size}/$(context) ; \
          $(INSTALL) $(top_srcdir)/icons/logo$$size.png $(themedir)/$${size}x$${size}/$(context)/gnubik.png ; \
	done 
	gtk-update-icon-cache --ignore-theme-index $(themedir)

INSTALL_DATA_HOOKS = install-icons

uninstall-icons:
	for size in $(sizes); do \
          $(RM) $(themedir)/$${size}x$${size}/$(context)/gnubik.png ; \
	done 
	gtk-update-icon-cache --ignore-theme-index $(themedir)

UNINSTALL_DATA_HOOKS = uninstall-icons

install-data-hook: $(INSTALL_DATA_HOOKS)
uninstall-hook: $(UNINSTALL_DATA_HOOKS)



EXTRA_DIST += icons/logo22.xcf icons/logo32.xcf icons/logo48.xcf
EXTRA_DIST += icons/logo16.png icons/logo22.png icons/logo32.png icons/logo48.png

desktopdir = $(datadir)/applications
desktop_DATA = icons/gnubik.desktop

EXTRA_DIST += icons/gnubik.desktop

