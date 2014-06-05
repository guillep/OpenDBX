AM_CPPFLAGS = -I../../lib

if SINGLELIB
noinst_LTLIBRARIES = libpgsqlbackend.la
libpgsqlbackend_la_CPPFLAGS = -DODBX_SINGLELIB
else
pkglib_LTLIBRARIES = libpgsqlbackend.la
libpgsqlbackend_la_CPPFLAGS =
libpgsqlbackend_la_LDFLAGS = -no-undefined -module -version-info @APIVERSION@
endif

libpgsqlbackend_la_SOURCES = pgsqlbackend.h pgsqlbackend.c pgsql_basic.h pgsql_basic.c
libpgsqlbackend_la_CPPFLAGS += -DAPINUMBER=@APINUMBER@ $(AM_CPPFLAGS)
libpgsqlbackend_la_LIBADD = -l@LIBPGSQL@
