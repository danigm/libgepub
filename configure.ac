AC_PREREQ(2.60)
AC_INIT([libgxps],[0.0.1],[danigm@wadobo.com])
AC_CONFIG_AUX_DIR([build])
AM_INIT_AUTOMAKE([1.10 -Wall -Werror dist-bzip2])

GNOME_COMMON_INIT

AC_PROG_CC
# Compiling sources with per-target flags requires AM_PROG_CC_C_O
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(GEPUB,
                  glib-2.0
                  gobject-2.0
                  gio-2.0
                  cairo
                  webkit-1.0
                  libxml-2.0
                  libarchive)

AC_SUBST(GEPUB_CFLAGS)
AC_SUBST(GEPUB_LIBS)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
    Makefile
    libgepub.pc
    libgepub/Makefile
    tests/Makefile
])

AC_OUTPUT
