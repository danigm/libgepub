lib_LTLIBRARIES = libgepub.la

NOINST_H_FILES = \
	gepub-arvhice.h

INST_H_FILES = \
	gepub.h

libgepubincludedir = $(includedir)/libgepub
libgepubinclude_HEADERS = $(INST_H_FILES)

libgepub_la_SOURCES = \
	gepub-archive.c			\
	$(NOINST_H_FILES)		\
	$(INST_H_FILES)

libgepub_la_CPPFLAGS = \
	-I$(top_builddir)	\
	-I$(srcdir)		\
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
