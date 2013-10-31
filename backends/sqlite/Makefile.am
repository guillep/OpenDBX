AM_CPPFLAGS = -I../../lib

if SINGLELIB
noinst_LTLIBRARIES = libsqlitebackend.la
libsqlitebackend_la_CPPFLAGS = -DODBX_SINGLELIB
else
pkglib_LTLIBRARIES = libsqlitebackend.la
libsqlitebackend_la_CPPFLAGS =
libsqlitebackend_la_LDFLAGS = -no-undefined -module -version-info @APIVERSION@
endif

libsqlitebackend_la_SOURCES = sqlitebackend.h sqlitebackend.c sqlite_basic.h sqlite_basic.c
libsqlitebackend_la_CPPFLAGS += -DAPINUMBER=@APINUMBER@ $(AM_CPPFLAGS)
libsqlitebackend_la_LIBADD = -l@LIBSQLITE@
