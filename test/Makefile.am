INCLUDES = -I../lib


if ENABLETEST

noinst_PROGRAMS = odbxtest odbxplustest

odbxtest_SOURCES = odbx-regression.c odbx-regression.h
odbxtest_LDADD = ../lib/$(LIBPREFIX)opendbx.la
odbxtest_LDFLAGS = -static

odbxplustest_SOURCES = odbxplus-regression.cpp
odbxplustest_LDADD = ../lib/$(LIBPREFIX)opendbx.la ../lib/$(LIBPREFIX)opendbxplus.la
odbxplustest_LDFLAGS = -static

endif


EXTRA_DIST = \
	odbxtest.site.template \
	odbxtest.sh \
	ref/firebird.ref \
	ref/mssql.ref \
	ref/mysql.ref \
	ref/odbc.ref \
	ref/oracle.ref \
	ref/pgsql.ref \
	ref/sqlite.ref \
	ref/sqlite3.ref \
	ref/sybase.ref
