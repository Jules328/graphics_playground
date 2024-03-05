#ifndef _GL_GLFW_H_
#define _GL_GLFW_H_

#include <stdio.h>

#include <GLFW/glfw3.h>

GLFWwindow* window = NULL;

void error_callback( int error, const char *msg ) {
    printf("[%d] %s\n", error, msg);
}

int glfw_init(void) {
    /* error callback for glfw issues */
    glfwSetErrorCallback(error_callback);

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    /* glfw window hints for opengl (1.3) profile */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "OpenGL 1.3 Height Map", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* make the window's context current */
    glfwMakeContextCurrent(window);

    /* cap framerate at screen refresh */
    glfwSwapInterval(1);

    return 0;
}

void glfw_set_key_callback(GLFWkeyfun callback) {
    glfwSetKeyCallback(window, callback);
}

void glfw_set_mouse_button_callback(GLFWmousebuttonfun callback) {
    glfwSetMouseButtonCallback(window, callback);
}

void glfw_set_scroll_callback(GLFWscrollfun callback) {
    glfwSetScrollCallback(window, callback);
}

void glfw_set_cursor_pos_callback(GLFWcursorposfun callback) {
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, callback);
}

int glfw_should_close(void) {
    return glfwWindowShouldClose(window);
}

void glfw_get_window_size(int* width, int* height) {
    glfwGetFramebufferSize(window, width, height);
}

#endif /* _GL_GLFW_H_ */