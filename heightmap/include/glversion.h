#ifndef _GLVERSION_H_
#define _GL_VERSION_H_

#include <GLFW/glfw3.h>

#if _GL_VERSION_ == 2
#define USE_GL2
#else
#define USE_GL1
#endif

#ifdef USE_GL1
#include <GL/gl.h>
#define GL_CONTEXT_VERSION_MAJOR 1
#define GL_CONTEXT_VERSION_MINOR 3
#define GL_CLIENT_API GLFW_OPENGL_API
#endif

#ifdef USE_GL2
#include <GLES2/gl2.h>
#define GL_CONTEXT_VERSION_MAJOR 2
#define GL_CONTEXT_VERSION_MINOR 0
#define GL_CLIENT_API GLFW_OPENGL_ES_API
#endif

#endif /* _GL_VERSION_H_ */