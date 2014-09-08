AC_PREREQ(2.60)
AC_INIT([libgepub],[0.2],[danigm@wadobo.com])
AC_CONFIG_AUX_DIR([build])
AM_INIT_AUTOMAKE([1.10 -Wall -Werror dist-bzip2])

GNOME_COMMON_INIT

# automake 1.12 seems to require this, but automake 1.11 doesn't recognize it
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

AC_PROG_CC
# Compiling sources with per-target flags requires AM_PROG_CC_C_O
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(GEPUB,
                  glib-2.0
                  gobject-2.0
                  gtk+-3.0
                  gio-2.0
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

# introspection
GOBJECT_INTROSPECTION_CHECK([1.30.0])

AC_OUTPUT
