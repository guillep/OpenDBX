if ENABLEUTILS

SUBDIRS = doc po

INCLUDES = -I../lib

bin_PROGRAMS = odbx-sql

odbx_sql_SOURCES = argmap.cpp argmap.hpp commands.cpp commands.hpp completion.cpp completion.hpp odbx-sql.hpp odbx-sql.cpp
odbx_sql_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" -DKEYWORDFILE=\"$(pkgdatadir)/keywords\"
odbx_sql_LDADD = ../lib/.libs/$(LIBPREFIX)opendbx$(LIBSUFFIX) ../lib/.libs/$(LIBPREFIX)opendbxplus$(LIBSUFFIX) $(LTLIBINTL) -lreadline -lncurses

EXTRA_DIST = keywords
pkgdata_DATA = keywords

endif
