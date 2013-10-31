include_HEADERS = odbx.h
nobase_include_HEADERS = opendbx/api.h opendbx/api


lib_LTLIBRARIES = libopendbx.la libopendbxplus.la

libopendbx_la_SOURCES =  odbxlib.c odbxlib.h odbx.c odbxdrv.h
libopendbx_la_CPPFLAGS = -DLIBVERSION=@LIBVERSION@ -DLOCALEDIR=\"$(localedir)\"
libopendbx_la_LDFLAGS = -no-undefined -version-info @APIVERSION@
libopendbx_la_LIBADD = $(MAINLIBS) $(LTLIBINTL)

libopendbxplus_la_SOURCES =  odbx_iface.hpp odbx_impl.hpp odbx_impl.cpp odbx.cpp
libopendbxplus_la_CXXFLAGS = -DLIBVERSION=@LIBVERSION@
libopendbxplus_la_LDFLAGS = -no-undefined -version-info @APIPLUSVERSION@
libopendbxplus_la_LIBADD = $(LTLIBINTL) libopendbx.la


if SINGLELIB

libopendbx_la_CPPFLAGS += -DODBX_SINGLELIB
libopendbx_la_LIBADD += @backendlibs@

else

libopendbx_la_CPPFLAGS += -DLIBPATH=\"$(pkglibdir)\" -DLIBPREFIX=\"@LIBPREFIX@\" -DLIBSUFFIX=\"@LIBSUFFIX@\"

endif
