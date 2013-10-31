AM_CPPFLAGS = -I../../lib

if SINGLELIB
noinst_LTLIBRARIES = libfirebirdbackend.la
libfirebirdbackend_la_CPPFLAGS = -DODBX_SINGLELIB
else
pkglib_LTLIBRARIES = libfirebirdbackend.la
libfirebirdbackend_la_CPPFLAGS =
libfirebirdbackend_la_LDFLAGS = -no-undefined -module -version-info @APIVERSION@
endif

libfirebirdbackend_la_SOURCES = firebirdbackend.h firebirdbackend.c firebird_basic.h firebird_basic.c firebird_lo.h firebird_lo.c
libfirebirdbackend_la_CPPFLAGS += -DAPINUMBER=@APINUMBER@ $(AM_CPPFLAGS)
libfirebirdbackend_la_LIBADD = -l@LIBFIREBIRD@
