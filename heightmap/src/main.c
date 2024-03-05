#include <string.h> /* memset */

#include "heightmap.h"
#include "window.h"

/* pre allocates memory */
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

void gen_vertices(uint32_t height, uint32_t width, float spacing, float scale) {
    uint32_t h, w;
    float h_offset = (height - 1) * spacing / 2.0f;
    float w_offset = (width - 1) * spacing / 2.0f;

    /* Clears memory */
    memset(&vertices[0][0][0], 0, sizeof(vertices));
    memset(&colors[0][0][0], 0, sizeof(colors));

    for (h = 0; h < height; ++h) {
        for (w = 0; w < width; ++w) {
            /* xy plane heightmap */
            // vertices[h][w][0] = spacing * w - w_offset; /* -x in top left */
            // vertices[h][w][1] = h_offset - spacing * h; /* +y in top left */
            // vertices[h][w][2] = heightmap_pixels[h][w] * scale;

            /* xz plane heightmap - y is altitude, -z is "north", +x is "east"*/
            vertices[h][w][0] = spacing * w - w_offset; /* -x in far left */
            vertices[h][w][2] = spacing * h - h_offset; /* -z in far left */
            vertices[h][w][1] = heightmap_pixels[h][w] * scale;

            colors[h][w][0] = heightmap_pixels[h][w];
            colors[h][w][1] = heightmap_pixels[h][w];
            colors[h][w][2] = heightmap_pixels[h][w];
        }
    }
}

int main(void)
{
    /* init window to draw to */
    if (window_init() == -1) 
        return -1;

    /* Setup vertex and color arrays */
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &vertices[0][0][0]);
    glColorPointer(3, GL_UNSIGNED_BYTE, 0, &colors[0][0][0]);
    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); /* wireframe */

    /* clears vertices memory */
    gen_vertices(HEIGHTMAP_HEIGHT, HEIGHTMAP_WIDTH, 0.1f, 0.01f);
    gen_indices(HEIGHTMAP_HEIGHT, HEIGHTMAP_WIDTH);

    /* starts the window logic */
    window_start();

    /* Loop until the user closes the window */
    while (!window_should_close()) {
        /* updates window size and camera variables */
        window_update();

        /* draw new elements */
        glDrawElements(GL_TRIANGLE_STRIP, INDICES, GL_UNSIGNED_INT, &indices[0]);

        /* draws the frame and checks for draw errors */
        if (window_draw_frame() == -1) {
            break;
        }
    }

    glfwTerminate();
    return 0;
}
