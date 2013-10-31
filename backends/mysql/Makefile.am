AM_CPPFLAGS = -I../../lib

if SINGLELIB
noinst_LTLIBRARIES = libmysqlbackend.la
libmysqlbackend_la_CPPFLAGS = -DODBX_SINGLELIB
else
pkglib_LTLIBRARIES = libmysqlbackend.la
libmysqlbackend_la_CPPFLAGS =
libmysqlbackend_la_LDFLAGS = -no-undefined -module -version-info @APIVERSION@
endif

libmysqlbackend_la_SOURCES = mysqlbackend.h mysqlbackend.c mysql_basic.h mysql_basic.c
libmysqlbackend_la_CPPFLAGS += -DAPINUMBER=@APINUMBER@ $(AM_CPPFLAGS)
libmysqlbackend_la_LIBADD = -l@LIBMYSQL@
