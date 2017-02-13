lib_LTLIBRARIES = libgepub.la

INST_H_FILES = \
	gepub-widget.h		\
	gepub-doc.h		\
	gepub.h

libgepubincludedir = $(includedir)/libgepub
libgepubinclude_HEADERS = $(INST_H_FILES)

libgepub_la_SOURCES = \
	gepub-widget.c			\
	gepub-doc.c				\
	$(NOINST_H_FILES)		\
	$(INST_H_FILES)


RUST_SOURCES =					\
	rust/Cargo.toml				\
	rust/src/lib.rs

if DEBUG_RELEASE
CARGO_RELEASE_ARGS=
else
CARGO_RELEASE_ARGS=--release
endif

RUST_LIB=@abs_top_builddir@/target/@RUST_TARGET_SUBDIR@/libepub_internals.a

check-local:
	cd $(srcdir)/rust && \
	CARGO_TARGET_DIR=@abs_top_builddir@/target cargo test

clean-local:
	cd $(top_srcdir)/rust && \
	CARGO_TARGET_DIR=@abs_top_builddir@/target cargo clean

@abs_top_builddir@/target/@RUST_TARGET_SUBDIR@/libepub_internals.a: $(RUST_SOURCES)
	cd $(top_srcdir)/libgepub/rust && \
	CARGO_TARGET_DIR=@abs_top_builddir@/target cargo build --verbose $(CARGO_RELEASE_ARGS)

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
	$(GEPUB_LIBS)	 \
	$(RUST_LIB)

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
introspection_sources = $(libgepub_la_SOURCES)

Gepub-0.4.gir: libgepub.la
Gepub_0_4_gir_INCLUDES = GObject-2.0 libxml2-2.0 WebKit2-4.0
Gepub_0_4_gir_CFLAGS = $(INCLUDES)
Gepub_0_4_gir_LIBS = libgepub.la
Gepub_0_4_gir_FILES = $(introspection_sources)
Gepub_0_4_gir_SCANNERFLAGS = --warn-all
INTROSPECTION_GIRS += Gepub-0.4.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES = $(gir_DATA) $(typelib_DATA)
endif
