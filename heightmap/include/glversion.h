#ifndef _GLVERSION_H_
#define _GL_VERSION_H_

#include <stdio.h>

#include <cglm/cglm.h>
#include <GLFW/glfw3.h>

/* defaults to OpenGL 2 */
#if _GL_VERSION_ == 1
#define USE_GL1
#else
#define USE_GL2
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

int success;
char infoLog[512];
GLuint fragShader, vertShader, shaderProgram;
GLint u_mvp, a_pos, a_tex;
const char* vert_shader =
"uniform sampler2D heightmap;\n"
"uniform float alt_scale;\n"
"uniform mat4 u_mvp;\n"
"\n"
"attribute vec2 a_pos;\n"
"attribute vec2 a_tex;\n"
"\n"
"varying vec3 v_col;\n"
// "varying vec2 v_tex;\n"
"void main() {\n"
// "   v_tex = a_tex;\n"
"   float height = texture2D(heightmap, a_tex).r;\n"
"   v_col = vec3(height);\n"
// "   v_col = vec3(a_tex, 0.0);\n"
// "   v_col = texture2D(heightmap, a_tex).rgb;\n"
"   gl_Position = u_mvp * vec4(a_pos, alt_scale * height , 1.0);\n"
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
// "uniform sampler2D heightmap;\n"
"varying vec3 v_col;\n"
// "varying vec2 v_tex;\n"
"void main() {\n"
// "   gl_FragColor = texture2D(heightmap, v_tex);\n"
"   gl_FragColor = vec4(v_col, 1.0f);\n"
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
#endif

int gl_init(void) {
    /* set clear color and enable depth testing */
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glEnable(GL_DEPTH_TEST);
    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); /* wireframe */

#ifdef USE_GL1
    /* Setup vertex and color arrays */
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
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
        return -1;
    }

    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &frag_shader, NULL);
    glCompileShader(fragShader);
    // check for issues
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        printf("ERROR Fragment Shader Compilation Failed: %s\n", infoLog);
        return -1;
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
        return -1;
    }

    /* use program */
    glUseProgram(shaderProgram);

    /* remove unneeded shaders - not SC */
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    /* Grab locations of uniforms and attributes */
    u_mvp = glGetUniformLocation(shaderProgram, "u_mvp");
    a_pos = glGetAttribLocation(shaderProgram, "a_pos");
    a_tex = glGetAttribLocation(shaderProgram, "a_tex");
    printf("u_mvp: %d, a_pos: %d, a_tex: %d\n", u_mvp, a_pos, a_tex);
    /* TODO - add check for if any uniforms or attributes are -1 */

    /* TODO - replace with buffer object */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(a_pos);
    glEnableVertexAttribArray(a_tex);
#endif

    return 0;
}

void gl_window_update(int width, int height, vec4 *view, vec4* proj) {
    /* clear frame */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* resizes viewport to current window size */
    glViewport(0, 0, width, height);

#ifdef USE_GL1
    /* Update projection matrix to fit window */
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&proj[0][0]);

    /* Update model matrix to rotate */
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&view[0][0]);
#endif
#ifdef USE_GL2
    /* create mvp matrices from view and projection */
    mat4 mvp;
    glm_mat4_mul(proj, view, mvp);
    glUniformMatrix4fv(u_mvp, 1, false, &mvp[0][0]);
#endif
}

int gl_draw_frame(void) {
    uint32_t error = glGetError();
    if (error != 0) {
        printf("OpenGL errorcode 0x%04x\n", error);
        return -1;
    }

    return 0;
}

#endif /* _GL_VERSION_H_ */