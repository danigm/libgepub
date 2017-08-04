SUBDIRS = libgepub tests

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgepub.pc

# introspection
DISTCHECK_CONFIGURE_FLAGS = --enable-introspection

EXTRA_DIST = \
	$(pkgconfig_DATA).in	\
	meson.build		\
	meson_options.txt

DISTCLEANFILES = \
	$(pkgconfig_DATA)
