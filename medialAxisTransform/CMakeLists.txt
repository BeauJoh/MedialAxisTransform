include_directories(${MedialAxisTransform_SOURCE_DIR}/MedialAxisTransform)
link_directories(${MedialAxisTransform_BINARY_DIR}/MedialAxisTransform)

set(CORELIBS ${GLUT_LIBRARY} ${OPENGL_LIBRARY} ${OPENCL_LIBRARY} ${PNG_LIBRARY} m)

add_library(FileHandler FileHandler.cpp)
add_library(marchingcubes marchingcubes.cpp)
add_library(openCLUtilities openCLUtilities.cpp)
add_library(openGLVisualiser openGLVisualiser.cpp)
add_library(RGBAUtilities RGBAUtilities.cpp)
add_library(vec3f vec3f.cpp)


add_executable(MedialAxisTransform main.cpp)
#linking against a custom library
target_link_libraries(MedialAxisTransform ${CORELIBS} FileHandler marchingcubes openCLUtilities  openGLVisualiser RGBAUtilities vec3f)

