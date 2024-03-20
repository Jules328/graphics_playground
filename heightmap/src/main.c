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

int success;
char infoLog[512];
GLuint fragShader, vertShader, shaderProgram;
GLint u_mvp, a_pos, a_col;
const char* vert_shader =
"uniform mat4 u_mvp;\n"
"attribute vec3 a_pos;\n"
"attribute vec3 a_col;\n"
"varying vec3 v_col;\n"
"void main() {\n"
"   v_col = a_col;\n"
"   gl_Position = u_mvp * vec4(a_pos, 1.0);\n"
"}\0";
// "uniform mat4 mvp_matrix; // model-view-projection matrix"
// "uniform mat3 normal_matrix; // normal matrix"
// "uniform vec3 ec_light_dir; // light direction in eye coords"
// "attribute vec4 a_vertex; // vertex position"
// "attribute vec3 a_normal; // vertex normal"
// "attribute vec2 a_texcoord; // texture coordinates"
// "varying float v_diffuse;"
// "varying vec2 v_texcoord;"
// "void main(void) {"
// "   // put vertex normal into eye coords"
// "   vec3 ec_normal = normalize(normal_matrix * a_normal);"
// "   // emit diffuse scale factor, texcoord, and position"
// "   v_diffuse = max(dot(ec_light_dir, ec_normal), 0.0);"
// "   v_texcoord = a_texcoord;"
// "   gl_Position = mvp_matrix * a_vertex;"
// "}";

const char* frag_shader =
"precision mediump float;\n"
"varying vec3 v_col;\n"
"void main() {\n"
"    gl_FragColor = vec4(v_col, 1.0f);\n"
"}\0";
// "precision mediump float;"
// "uniform sampler2D t_reflectance;"
// "uniform vec4 i_ambient;"
// "varying float v_diffuse;"
// "varying vec2 v_texcoord;"
// "void main (void) {"
// "   vec4 color = texture2D(t_reflectance, v_texcoord);"
// "   gl_FragColor = color * (vec4(v_diffuse) + i_ambient);"
// "}";

int main(void)
{
    /* init window to draw to */
    if (window_init() == -1) 
        return -1;

    /* Generates vertices and indices to draw */
    gen_vertices(HEIGHTMAP_HEIGHT, HEIGHTMAP_WIDTH, 0.1f, 0.01f);
    gen_indices(HEIGHTMAP_HEIGHT, HEIGHTMAP_WIDTH);

#ifdef USE_GL1
    /* Setup vertex and color arrays */
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &vertices[0][0][0]);
    glColorPointer(3, GL_FLOAT, 0, &colors[0][0][0]);
    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); /* wireframe */
#endif
#ifdef USE_GL2
    /* compile vertex and fragment shaders - not SC */
    vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vert_shader, NULL);
    glCompileShader(vertShader);
    // check for issues
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
        printf("ERROR Vertex Shader Compilation Failed: %s\n", infoLog);
    }

    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &frag_shader, NULL);
    glCompileShader(fragShader);
    // check for issues
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        printf("ERROR Fragment Shader Compilation Failed: %s\n", infoLog);
    }

    /* create program, attach shaders, and link - creat and use are SC */
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);
    // check for issues
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR Program Linking Failed: %s\n", infoLog);
    }

    /* use program */
    glUseProgram(shaderProgram);

    /* remove unneeded shaders - not SC */
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    /* Grab locations of uniforms and attributes */
    u_mvp = glGetUniformLocation(shaderProgram, "u_mvp");
    a_pos = glGetAttribLocation(shaderProgram, "a_pos");
    a_col = glGetAttribLocation(shaderProgram, "a_col");
    printf("u_mvp: %d, a_pos: %d, a_col: %d\n", u_mvp, a_pos, a_col);

    /* TODO - replace with buffer object */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(a_pos, 3, GL_FLOAT, false, 0, &vertices[0][0][0]);
    glEnableVertexAttribArray(a_pos);
    glVertexAttribPointer(a_col, 3, GL_FLOAT, false, 0, &colors[0][0][0]);
    glEnableVertexAttribArray(a_col);
#endif

    /* starts the window logic */
    window_start();

    /* Loop until the user closes the window */
    while (!window_should_close()) {
        /* updates window size and camera variables */
        window_update();

#ifdef USE_GL1
        /* draw elements */
        glDrawElements(GL_TRIANGLE_STRIP, NUM_INDICES, GL_UNSIGNED_INT, &indices[0]);
#endif
#ifdef USE_GL2
        /* draw elements - temporary */
        glDrawElements(GL_TRIANGLE_STRIP, NUM_INDICES, GL_UNSIGNED_INT, &indices[0]);
#endif

        /* draws the frame and checks for draw errors */
        if (window_draw_frame() == -1) {
            break;
        }
    }

    glfwTerminate();
    return 0;
}
