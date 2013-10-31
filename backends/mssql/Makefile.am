AM_CPPFLAGS = -I../../lib

if SINGLELIB
noinst_LTLIBRARIES = libmssqlbackend.la
libmssqlbackend_la_CPPFLAGS = -DODBX_SINGLELIB
else
pkglib_LTLIBRARIES = libmssqlbackend.la
libmssqlbackend_la_CPPFLAGS =
libmssqlbackend_la_LDFLAGS = -no-undefined -module -version-info @APIVERSION@
endif

libmssqlbackend_la_SOURCES = mssqlbackend.h mssqlbackend.c mssql_basic.h mssql_basic.c
libmssqlbackend_la_CPPFLAGS += -DAPINUMBER=@APINUMBER@ $(AM_CPPFLAGS)
libmssqlbackend_la_LIBADD = -l@LIBMSSQL@
