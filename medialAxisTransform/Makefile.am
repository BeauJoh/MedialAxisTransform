ACLOCAL_AMFLAGS= -I m4
AUTOMAKE_OPTIONS= gnu
AM_CPPFLAGS=

bin_PROGRAMS=MedialAxisTransform

MedialAxisTransform_SOURCES=\
	main.cpp\
	FileHandler.cpp\
	openCLUtilities.cpp\
	RGBAUtilities.cpp\
	openGLVisualiser.cpp

noinst_HEADERS=\
 	openCLUtilities.h\
	RGBAUtilities.h\
	openGLVisualiser.h

#if defined (__APPLE__)  && defined (__MACH__)
MedialAxisTransform_LDFLAGS= -L/usr/X11/lib -L/System/Library/Frameworks/OpenCL.framework/Libraries -framework GLUT -framework OpenGL -framework OpenCL -framework libpng
#else
MedialAxisTransform_LDFLAGS= -lGLUT -lOpenGL -lOpenCL -lpng
#endif


