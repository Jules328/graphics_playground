#include <string.h> /* memset */

#include "heightmap.h"
#include "test_texture.h"
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

#ifdef USE_GL1
/* pre allocates memory */
vec3 vertices[HEIGHTMAP_HEIGHT][HEIGHTMAP_WIDTH];
vec3 colors[HEIGHTMAP_HEIGHT][HEIGHTMAP_WIDTH];

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
#endif
#ifdef USE_GL2
GLuint texture;

float alt_scale = 1.f;

vec2 vertices[HEIGHTMAP_HEIGHT][HEIGHTMAP_WIDTH];
vec2 tex_coords[HEIGHTMAP_HEIGHT][HEIGHTMAP_WIDTH];

void gen_vertices(uint32_t height, uint32_t width, float spacing) {
    uint32_t h, w;
    float h_offset = (height - 1) * spacing / 2.0f;
    float w_offset = (width - 1) * spacing / 2.0f;

    /* Clears memory */
    memset(&vertices[0][0][0], 0, sizeof(vertices));

    for (h = 0; h < height; ++h) {
        for (w = 0; w < width; ++w) {
            /* xy plane heightmap - +z is altitude, +x is east, +y is north */
            vertices[h][w][0] = spacing * w - w_offset; /* -x in top left */
            vertices[h][w][1] = h_offset - spacing * h; /* +y in top left */
        }
    }
}

void gen_tex_coords(uint32_t height, uint32_t width) {
    uint32_t h, w;
    float s_initial = 0.f;
    float s_scale = 1.f  / ((float) width - 1);
    float t_initial = 0.f;
    float t_scale = 1.f  / ((float) height - 1);

    /* Clears memory */
    memset(&tex_coords[0][0][0], 0, sizeof(tex_coords));

    for (h = 0; h < height; ++h) {
        for (w = 0; w < width; ++w) {
            tex_coords[h][w][0] = s_initial + w * s_scale; // s
            tex_coords[h][w][1] = t_initial + h * t_scale; // t
        }
    }
}
#endif

int main(void)
{
    /* init window to draw to */
    if (window_init() == -1) 
        return -1;

    /* generates common triangle strip mesh for terrain */
    gen_indices(HEIGHTMAP_HEIGHT, HEIGHTMAP_WIDTH);

#ifdef USE_GL1
    /* Generates vertices and colors to draw */
    gen_vertices(HEIGHTMAP_HEIGHT, HEIGHTMAP_WIDTH, 0.1f, 0.01f);

    /* Setup vertex and color arrays */
    glVertexPointer(3, GL_FLOAT, 0, &vertices[0][0][0]);
    glColorPointer(3, GL_FLOAT, 0, &colors[0][0][0]);
#endif
#ifdef USE_GL2
    /* generate terrain vertices */
    gen_vertices(HEIGHTMAP_HEIGHT, HEIGHTMAP_WIDTH, 0.1f);

    /* generate texture coordinates */
    gen_tex_coords(HEIGHTMAP_HEIGHT, HEIGHTMAP_WIDTH);

    /* set altitude scaling */
    glUniform1f(glGetUniformLocation(shaderProgram, "alt_scale"), alt_scale);

    /* create and fill texture with heightmap data */
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shaderProgram, "heightmap"), 0); // zero relates to texturing unit

    /* texture params */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* fill in texture */
    /* NOTE: had to be power of two to get a correct texturing */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, HEIGHTMAP_WIDTH, HEIGHTMAP_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, &heightmap_pixels[0][0]); // not SC - need to replace with glTexStorage2D
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TEST_TEXTURE_WIDTH, TEST_TEXTURE_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, &test_texture_pixels[0][0]);

    /* TODO - replace with buffer object */
    glVertexAttribPointer(a_pos, 2, GL_FLOAT, false, 0, &vertices[0][0][0]);
    glVertexAttribPointer(a_tex, 2, GL_FLOAT, false, 0, &tex_coords[0][0][0]);
#endif

    /* starts the window logic */
    window_start();

    /* Loop until the user closes the window */
    while (!window_should_close()) {
        /* updates window size and camera variables */
        window_update();

#ifdef USE_GL2
        /* up */
        if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
            alt_scale -= 0.25f;
        }
        /* down */
        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
            alt_scale += 0.25f;
        }

        /* set altitude scaling */
        glUniform1f(glGetUniformLocation(shaderProgram, "alt_scale"), alt_scale);
#endif

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
