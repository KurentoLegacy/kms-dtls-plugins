dnl required version of autoconf
AC_PREREQ([2.53])

AC_INIT([kms-dtls-plugins],[1.1.0-dev])

dnl required versions of gstreamer and plugins-base
GST_REQUIRED=1.2.0
GLIB_REQ=2.38
GNUTLS_MIN_REQUIRED=3.2.0

SOUP_REQUIRED=2.40
NICE_REQUIRED=0.1.4

AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])

dnl required version of automake
AM_INIT_AUTOMAKE([1.10])

GETTEXT_PACKAGE=kms-dtls

AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[The gettext domain name])
AM_GLIB_GNU_GETTEXT

dnl enable mainainer mode by default
AM_MAINTAINER_MODE([enable])

dnl check for tools (compiler etc.)
AC_PROG_CC
AC_PROG_CC_STDC

dnl check if the compiler supports '-c' and '-o' options
AM_PROG_CC_C_O

dnl required version of libtool
LT_PREREQ([2.2.6])
LT_INIT

AC_SUBST(ACLOCAL_AMFLAGS, "-I m4")

AG_GST_GLIB_CHECK([$GLIB_REQ])

dnl give error and exit if we don't have pkgconfig
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, [ ], [
  AC_MSG_ERROR([You need to have pkg-config installed!])
])

PKG_CHECK_MODULES(GST, [
  gstreamer-1.0 >= $GST_REQUIRED
], [
], [
  AC_MSG_ERROR([
      You need to install or upgrade the GStreamer development
      packages on your system. On debian-based systems these are
      libgstreamer1.0-dev and libgstreamer-plugins-base1.0-dev.
      on RPM-based systems gstreamer1.0-devel, libgstreamer1.0-devel
      or similar. The minimum version required is $GST_REQUIRED.
  ])
])

PKG_CHECK_MODULES(GIO, [
  gio-2.0 >= $GLIB_REQUIRED
], [
], [
  AC_MSG_ERROR([
      You need to install or upgrade the Gio development
      packages on your system to version $GIO_REQUIRED.
  ])
])

PKG_CHECK_MODULES(GST_CHECK, [
  gstreamer-check-1.0 >= $GST_REQUIRED
], [
  HAVE_GST_CHECK="yes"
], [
  HAVE_GST_CHECK="no"
  AC_MSG_NOTICE([no gstreamer-check-1.0 >= $GST_REQUIRED (GStreamer Check unittest Library) found])
])
AM_CONDITIONAL(HAVE_GST_CHECK, test "x$HAVE_GST_CHECK" = "xyes")

PKG_CHECK_MODULES(SOUP, [
  libsoup-2.4 >= $SOUP_REQUIRED
], [
  AC_SUBST(SOUP_CFLAGS)
  AC_SUBST(SOUP_LIBS)
  soup_found=true
], [
  AC_MSG_WARN([
    Lib soup not found, some test are not compiled.
  ])
  soup_found=false
])
AM_CONDITIONAL([HAS_SOUP], [test x$soup_found = xtrue])

PKG_CHECK_MODULES(NICE, [
  nice >= $NICE_REQUIRED
], [
  AC_SUBST(NICE_CFLAGS)
  AC_SUBST(NICE_LIBS)
  nice_found=true
], [
  AC_MSG_WARN([
    libnice not found, some test are not compiled.
  ])
  nice_found=false
])
AM_CONDITIONAL([HAS_NICE], [test x$nice_found = xtrue])

dnl check if compiler understands -Wall (if yes, add -Wall to GST_CFLAGS)
AC_MSG_CHECKING([to see if compiler understands -Wall])
save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Wall"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([ ], [ ])], [
  GST_CFLAGS="$GST_CFLAGS -Wall"
  AC_MSG_RESULT([yes])
], [
  AC_MSG_RESULT([no])
])

PKG_CHECK_MODULES(GNUTLS, [
  gnutls >= $GNUTLS_MIN_REQUIRED
], [
], [
  AC_MSG_ERROR([
      You need to install or upgrade gnutls.
      The minimum version required is $GNUTLS_MIN_REQUIRED (libgnutls28-dev).
  ])
])

AC_SUBST([LIBGNUTLS_CFLAGS])
AC_SUBST([LIBGNUTLS_LIBS])

AC_MSG_CHECKING(for certtool)
if test "x$CERTTOOL_GNUTLS" != "x"; then
  AC_MSG_RESULT([$CERTTOOL_GNUTLS (from environment)])
else
  AC_PATH_PROG(CERTTOOL_GNUTLS, [certtool], [certtool])
  AC_MSG_RESULT([$CERTTOOL_GNUTLS])
fi
if ! $CERTTOOL_GNUTLS --version 2>/dev/null >/dev/null; then
  AC_MSG_ERROR([$CERTTOOL_GNUTLS does not seem to work!])
fi
AC_SUBST(CERTTOOL_GNUTLS)

dnl set the plugindir where plugins should be installed (for src/Makefile.am)
if test "x${prefix}" = "x$HOME"; then
  plugindir="$HOME/.kms-dtls-plugins/plugins"
else
  plugindir="\$(libdir)/kms-dtls-plugins"
fi
AC_SUBST(plugindir)

dnl set proper LDFLAGS for plugins
GST_PLUGIN_LDFLAGS='-module -avoid-version -export-symbols-regex [_]*\(gst_\|Gst\|GST_\).*'
AC_SUBST(GST_PLUGIN_LDFLAGS)

if test "x${FINAL_INSTALL_DIR}" = "x"; then
  AC_DEFINE([FINAL_INSTALL_DIR], [""],"Final installation Path")
else
  AC_DEFINE_UNQUOTED([FINAL_INSTALL_DIR], ["$FINAL_INSTALL_DIR"],"Final installation Path")
fi

AC_CONFIG_FILES([Makefile src/Makefile
                          tests/Makefile
                          tests/check/Makefile
                          tests/examples/Makefile])
AC_OUTPUT
