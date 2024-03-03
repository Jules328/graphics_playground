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

#define INDICES (HEIGHTMAP_NUM_PIXELS + (HEIGHTMAP_HEIGHT-2) * (HEIGHTMAP_WIDTH-1))
uint32_t indices[INDICES];

void gen_indices(uint32_t height, uint32_t width) {
    uint32_t i, h = 0, w = 0, points;
    int firststep = 1, step = 0, direction = 1;

    /* Clears memory */
    memset(&indices[0], 0, sizeof(indices));

    points = height*width + (height-2)*(width-1);
    for (i = 0; i < points; ++i) {
        indices[i] = h * width + w;

        if (step == 0) { /* downward motion of triangulation */
            ++h;
            step = 1;
        } else if (step == 1) { /* upward and sideways motion of triangulation */
            --h;
            w += direction; /* moves in the current direction */
            step = 0;
        }

        /* if at the endges of the grid */
        if (w == 0 || w == width - 1) {
            /* if downward motion was just taken - row was just completed */
            if (!firststep && step == 1) {
                step = 0; /* move down again */
                direction *= -1; /* swap direction */
                firststep = 1;
            } else {
                firststep = 0;
            }
        }
    }
}

void gen_vertices(uint32_t height, uint32_t width, float spacing) {
    uint32_t h, w;
    float h_offset = (height - 1) * spacing / 2.0f;
    float w_offset = (width - 1) * spacing / 2.0f;

    /* Clears memory */
    memset(&vertices[0][0][0], 0, sizeof(vertices));
    memset(&colors[0][0][0], 0, sizeof(colors));

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
    double start_time, current_time, delta_time;
    uint32_t frames = 0;

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
    gen_vertices(HEIGHTMAP_HEIGHT, HEIGHTMAP_WIDTH, 0.01f);
    gen_indices(HEIGHTMAP_HEIGHT, HEIGHTMAP_WIDTH);

    start_time = glfwGetTime();
    current_time = start_time;
    delta_time = 0.0;

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
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef((float) current_time * 180.0f / M_PI, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glDrawElements(GL_TRIANGLE_STRIP, INDICES, GL_UNSIGNED_INT, &indices[0]);

        /* check GL errors */
        uint32_t error = glGetError();
        if (error != 0) {
            printf("OpenGL errorcode 0x%04x\n", error);
            break;
        }
       
        glfwSwapBuffers(window);
        glfwPollEvents();

        /* Update timing */
        delta_time = glfwGetTime() - current_time;
        current_time = glfwGetTime();
        if (++frames % 60 == 0) {
            printf("Average frames per second: %5.2f\n", frames / (current_time - start_time));
        }
    }

    glfwTerminate();
    return 0;
}
