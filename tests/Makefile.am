noinst_PROGRAMS = test-gepub

test_gepub_SOURCES = test-gepub.c
test_gepub_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libgepub \
	$(AM_CPPFLAGS)

test_gepub_CFLAGS = \
	$(GTK_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS)

test_gepub_LDADD = \
	$(top_builddir)/libgepub/libgepub.la \
	$(GTK_LIBS)
