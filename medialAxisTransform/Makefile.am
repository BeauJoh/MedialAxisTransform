ACLOCAL_AMFLAGS= -I m4
AUTOMAKE_OPTIONS= gnu
AM_CPPFLAGS= -framework GLUT -framework OpenGL -framework OpenCL -framework libpng -L/usr/X11/lib

bin_PROGRAMS=MedialAxisTransform

MedialAxisTransform_SOURCES=\
	main.cpp\
	FileHandler.cpp\
	openCLUtilities.cpp\
	RGBAUtilities.cpp\
	openGLVisualiser.cpp

noinst_HEADERS=\
	FileHander.h\
	openCLUtilities.h\
	RGBAUtilities.h\
	openGLVisualiser.h

MedialAxisTransform_LDADD= -lglut -lpng -lGL -lOpenCL

MedialAxisTransform_LDFLAGS= -L/usr/X11/lib
