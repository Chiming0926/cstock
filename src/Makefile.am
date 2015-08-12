AUTOMAKE_OPTIONS= foreign

bin_PROGRAMS 	= cstock 

cstock_CXXFLAGS = -I $(top_builddir)/inc -Wall \
				  -I $(top_builddir)/src/cdata 

cstock_SOURCES 	= main.cpp \
				  cdata/cdata.cpp \
				  cdata/bshtm.cpp

cstock_LDADD	= ../lib/http/lib_chttp.a \
				  $(top_builddir)/lib/img/lib_img.a
