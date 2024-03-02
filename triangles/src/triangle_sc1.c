#include <stdio.h>
#include <stdlib.h>

#include "linmath.h"

#include <GLSC1/glsc1.h>
#include <GLFW/glfw3.h>
 
static const vec2 vertices[] =
{
    { -0.6f, -0.4f },
    {  0.6f, -0.4f },
    {   0.f,  0.6f }
};

static const vec3 colors[] = {
    { 1.f, 0.f, 0.f },
    { 0.f, 1.f, 0.f },
    { 0.f, 0.f, 1.f }
};

static void error_callback( int error, const char *msg ) {
    printf("[%d] %s\n", error, msg);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void)
{
    GLFWwindow* window;

    /* error callback for glfw issues */
    glfwSetErrorCallback(error_callback);

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    /* glfw window hints for opengl profile */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* set key callback */
    glfwSetKeyCallback(window, key_callback);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* v-sync */
    glfwSwapInterval(1);

    /* set clear color */
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glEnable(GL_DEPTH_TEST); 

    /* Setup vertex and color arrays */
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, &vertices[0]);
    glColorPointer(3, GL_FLOAT, 0, &colors[0]);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float) height;
        glViewport(0, 0, width, height);

        /* Update projection matrix to fit window */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrthof(-ratio, ratio, -1.f, 1.f, -1.f, 1.f);

        /* Update model matrix to rotate */
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef((float) glfwGetTime() * 180.0f / M_PI, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // glDrawArrays(GL_TRIANGLES, 0, 3);
        glBegin(GL_TRIANGLES);
        glColor4f(1.f, 0.f, 0.f, 1.f); glVertex2f(-0.6f, -0.4f);
        glColor4f(0.f, 1.f, 0.f, 1.f); glVertex2f( 0.6f, -0.4f);
        glColor4f(0.f, 0.f, 1.f, 1.f); glVertex2f( 0.0f,  0.6f);
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
