
#ifndef FORCE_SERVER_GRAPHICS_HEADERS_H
#define FORCE_SERVER_GRAPHICS_HEADERS_H

// OpenGL includes
#if defined(WIN32) || defined(linux)
# include <GL/glut.h>
#elif defined(__APPLE__)
# include <GLUT/glut.h>
#endif

#endif
