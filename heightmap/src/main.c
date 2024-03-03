#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include "glfw.h"

#include "heightmap.h"

/* pre allocates memory */
typedef float vec3[3];
typedef uint8_t uvec3[3];
vec3 vertices[HEIGHTMAP_HEIGHT][HEIGHTMAP_WIDTH];
uvec3 colors[HEIGHTMAP_HEIGHT][HEIGHTMAP_WIDTH];

void gen_vertices(int height, int width, float spacing) {
    int h, w;
    float h_offset = (height - 1) * spacing / 2.0f;
    float w_offset = (width - 1) * spacing / 2.0f;
    for (h = 0; h < height; ++h) {
        for (w = 0; w < width; ++w) {
            vertices[h][w][0] = spacing * w - w_offset;
            vertices[h][w][1] = h_offset - spacing * h;
            vertices[h][w][2] = 0.f;

            colors[h][w][0] = heightmap_pixels[h][w];
            colors[h][w][1] = heightmap_pixels[h][w];
            colors[h][w][2] = heightmap_pixels[h][w];
        }
    }
}

int main(void)
{
    /* init glfw window */
    if (glfw_init() == -1) 
        return -1;

    /* set clear color */
    glClearColor(0.5, 0.5, 0.5, 1.0);
    // glEnable(GL_DEPTH_TEST);

    /* Setup vertex and color arrays */
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &vertices[0][0][0]);
    glColorPointer(3, GL_UNSIGNED_BYTE, 0, &colors[0][0][0]);

    /* clears vertices memory */
    memset(&vertices[0][0][0], 0, sizeof(vertices));
    memset(&colors[0][0][0], 0, sizeof(colors));
    gen_vertices(HEIGHTMAP_HEIGHT, HEIGHTMAP_WIDTH, 0.01f);

    /* Loop until the user closes the window */
    while (!glfw_should_close()) {
        /* resize openGL viewport to window's size */
        int width, height;
        glfw_get_window_size(&width, &height);
        glViewport(0, 0, width, height);

        /* Update projection matrix to fit window */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        const float ratio = 1.5f * width / (float) height;
        glOrtho(-ratio, ratio, -1.5f, 1.5f, -1.f, 1.f);

        /* Update model matrix to rotate */
        // glMatrixMode(GL_MODELVIEW);
        // glLoadIdentity();
        // glRotatef((float) glfwGetTime() * 180.0f / M_PI, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* drawing calls */
        glPointSize(2.5f);
        glEnable(GL_POINT_SMOOTH);
        glDrawArrays(GL_POINTS, 0, HEIGHTMAP_NUM_PIXELS);

        /* check GL errors */
        uint32_t error = glGetError();
        if (error != 0) {
            printf("OpenGL errorcode 0x%04x\n", error);
            break;
        }
       
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
