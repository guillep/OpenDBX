AM_CPPFLAGS = -I../../lib

if SINGLELIB
noinst_LTLIBRARIES = libodbcbackend.la
libodbcbackend_la_CPPFLAGS = -DODBX_SINGLELIB
else
pkglib_LTLIBRARIES = libodbcbackend.la
libodbcbackend_la_CPPFLAGS =
libodbcbackend_la_LDFLAGS = -no-undefined -module -version-info @APIVERSION@
endif

libodbcbackend_la_SOURCES = odbcbackend.h odbcbackend.c odbc_basic.h odbc_basic.c
libodbcbackend_la_CPPFLAGS += -DAPINUMBER=@APINUMBER@ $(AM_CPPFLAGS)
libodbcbackend_la_LIBADD = -l@LIBODBC@
