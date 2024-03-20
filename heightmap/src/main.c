#include <string.h> /* memset */

#include "heightmap.h"
#include "window.h"

/*         TODO list
 * -------------------------
 * - add terrain coloring and lighting (normal vectors, altitude map 16x16 texture?)
 * - draw further away locations at lower resolution
 * - draw only chunks of terrain
 * - choose what gets drawn (indices) based off of current location of camera 
 * - only draw triangles that will be seen (based on camera position and direction)
 * - allow for use of multiple heightmaps
 * - speed comparisons of opengl 1 version drawing everything (drawArrays), drawing specific indices (drawElements), and opengl 2 version
 * - make presentation - outline design of modern system and illustrate what is lost with OpenGL versions
 */

/* pre allocates memory */
vec3 vertices[HEIGHTMAP_HEIGHT][HEIGHTMAP_WIDTH];
vec3 colors[HEIGHTMAP_HEIGHT][HEIGHTMAP_WIDTH];

#define NUM_INDICES (HEIGHTMAP_NUM_PIXELS + (HEIGHTMAP_HEIGHT-2) * (HEIGHTMAP_WIDTH-1))
uint32_t indices[NUM_INDICES];

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

void gen_vertices(uint32_t height, uint32_t width, float spacing, float scale) {
    uint32_t h, w;
    float h_offset = (height - 1) * spacing / 2.0f;
    float w_offset = (width - 1) * spacing / 2.0f;

    /* Clears memory */
    memset(&vertices[0][0][0], 0, sizeof(vertices));
    memset(&colors[0][0][0], 0, sizeof(colors));

    for (h = 0; h < height; ++h) {
        for (w = 0; w < width; ++w) {
            uint8_t alt = heightmap_pixels[h][w];

            /* xy plane heightmap - z is altitude, +x is "east", +y is "north" */
            vertices[h][w][0] = spacing * w - w_offset; /* -x in top left */
            vertices[h][w][1] = h_offset - spacing * h; /* +y in top left */
            vertices[h][w][2] = alt * scale;

            colors[h][w][0] = alt / 255.f;
            colors[h][w][1] = alt / 255.f;
            colors[h][w][2] = alt / 255.f;
        }
    }
}

int main(void)
{
    /* init window to draw to */
    if (window_init() == -1) 
        return -1;

    /* Generates vertices and indices to draw */
    gen_vertices(HEIGHTMAP_HEIGHT, HEIGHTMAP_WIDTH, 0.1f, 0.01f);
    gen_indices(HEIGHTMAP_HEIGHT, HEIGHTMAP_WIDTH);

    /* Setup vertex and color arrays */
#ifdef USE_GL1
    glVertexPointer(3, GL_FLOAT, 0, &vertices[0][0][0]);
    glColorPointer(3, GL_FLOAT, 0, &colors[0][0][0]);
#endif
#ifdef USE_GL2
    /* TODO - replace with buffer object */
    glVertexAttribPointer(a_pos, 3, GL_FLOAT, false, 0, &vertices[0][0][0]);
    glVertexAttribPointer(a_col, 3, GL_FLOAT, false, 0, &colors[0][0][0]);
#endif

    /* starts the window logic */
    window_start();

    /* Loop until the user closes the window */
    while (!window_should_close()) {
        /* updates window size and camera variables */
        window_update();

        /* draw elements */
        glDrawElements(GL_TRIANGLE_STRIP, NUM_INDICES, GL_UNSIGNED_INT, &indices[0]);

        /* draws the frame and checks for draw errors */
        if (window_draw_frame() == -1) {
            break;
        }
    }

    glfwTerminate();
    return 0;
}
