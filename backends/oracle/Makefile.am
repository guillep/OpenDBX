AM_CPPFLAGS = -I../../lib

if SINGLELIB
noinst_LTLIBRARIES = liboraclebackend.la
liboraclebackend_la_CPPFLAGS = -DODBX_SINGLELIB
else
pkglib_LTLIBRARIES = liboraclebackend.la
liboraclebackend_la_CPPFLAGS =
liboraclebackend_la_LDFLAGS = -no-undefined -module -version-info @APIVERSION@
endif

liboraclebackend_la_SOURCES = oraclebackend.h oraclebackend.c oracle_basic.h oracle_basic.c oracle_lo.h oracle_lo.c
liboraclebackend_la_CPPFLAGS += -DAPINUMBER=@APINUMBER@ $(AM_CPPFLAGS)
liboraclebackend_la_LIBADD = -l@LIBORACLE@
