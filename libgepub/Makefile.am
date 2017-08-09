lib_LTLIBRARIES = libgepub.la

NOINST_H_FILES = \
	gepub-utils.h

INST_H_FILES = \
	gepub-widget.h		\
	gepub-archive.h		\
	gepub-text-chunk.h	\
	gepub-doc.h		\
	gepub.h

libgepubincludedir = $(includedir)/libgepub
libgepubinclude_HEADERS = $(INST_H_FILES)

libgepub_la_SOURCES = \
	gepub-widget.c			\
	gepub-archive.c			\
	gepub-text-chunk.c		\
	gepub-doc.c				\
	gepub-utils.c			\
	$(NOINST_H_FILES)		\
	$(INST_H_FILES)

libgepub_la_CPPFLAGS = \
	-I$(top_builddir)	\
	-I$(srcdir)			\
	-I$(top_srcdir)		\
	$(AM_CPPFLAGS)

libgepub_la_CFLAGS = \
	$(GEPUB_CFLAGS)	\
	$(WARN_CFLAGS)	\
	$(AM_CFLAGS)

libgepub_la_LDFLAGS = \
	-no-undefined						\
	-export-symbols-regex "^gepub_*"	\
	$(AM_LDFLAGS)

libgepub_la_LIBADD = \
	$(GEPUB_LIBS)

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
introspection_sources = $(libgepub_la_SOURCES)

Gepub-0.5.1.gir: libgepub.la
Gepub_0_5_1_gir_INCLUDES = GObject-2.0 libxml2-2.0 WebKit2-4.0
Gepub_0_5_1_gir_CFLAGS = $(INCLUDES)
Gepub_0_5_1_gir_LIBS = libgepub.la
Gepub_0_5_1_gir_FILES = $(introspection_sources)
Gepub_0_5_1_gir_SCANNERFLAGS = --warn-all
INTROSPECTION_GIRS += Gepub-0.5.1.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES = $(gir_DATA) $(typelib_DATA)
endif

EXTRA_DIST = meson.build
