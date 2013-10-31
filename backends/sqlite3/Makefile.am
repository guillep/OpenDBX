AM_CPPFLAGS = -I../../lib

if SINGLELIB
noinst_LTLIBRARIES = libsqlite3backend.la
libsqlite3backend_la_CPPFLAGS = -DODBX_SINGLELIB
else
pkglib_LTLIBRARIES = libsqlite3backend.la
libsqlite3backend_la_CPPFLAGS =
libsqlite3backend_la_LDFLAGS = -no-undefined -module -version-info @APIVERSION@
endif

libsqlite3backend_la_SOURCES = sqlite3backend.h sqlite3backend.c sqlite3_basic.h sqlite3_basic.c
libsqlite3backend_la_CPPFLAGS += -DAPINUMBER=@APINUMBER@ $(AM_CPPFLAGS)
libsqlite3backend_la_LIBADD = -l@LIBSQLITE3@
