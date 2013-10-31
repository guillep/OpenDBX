AM_CPPFLAGS = -I../../lib

if SINGLELIB
noinst_LTLIBRARIES = libsybasebackend.la
libsybasebackend_la_CPPFLAGS = -DODBX_SINGLELIB
else
pkglib_LTLIBRARIES = libsybasebackend.la
libsybasebackend_la_CPPFLAGS =
libsybasebackend_la_LDFLAGS = -no-undefined -module -version-info @APIVERSION@
endif

libsybasebackend_la_SOURCES = sybasebackend.h sybasebackend.c sybase_basic.h sybase_basic.c
libsybasebackend_la_CPPFLAGS += -DAPINUMBER=@APINUMBER@ $(AM_CPPFLAGS)
libsybasebackend_la_LIBADD = -l@LIBSYBASE@
