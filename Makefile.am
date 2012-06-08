SUBDIRS = libgepub tests

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgepub.pc

# introspection
DISTCHECK_CONFIGURE_FLAGS = --enable-introspection

EXTRA_DIST = \
	$(pkgconfig_DATA).in

DISTCLEANFILES = \
	$(pkgconfig_DATA)
