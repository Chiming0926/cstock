AUTOMAKE_OPTIONS= foreign

bin_PROGRAMS 	= cstock

opencv_link		= `pkg-config opencv --libs`

cstock_CXXFLAGS = -I $(top_builddir)/inc -Wall \
				  -I $(top_builddir)/src/cdata 

cstock_SOURCES 	= main.cpp \
				  cdata/cdata.cpp \
				  cdata/bshtm.cpp

cstock_LDADD	= $(top_builddir)/lib/http/lib_chttp.a \
				  $(top_builddir)/lib/img/lib_cimg.a \
				  -lopencv_core \
				  -lopencv_highgui \
				  -lopencv_imgproc \
				  -lopencv_ml \
				  -lopencv_gpu \
				  -lopencv_objdetect
