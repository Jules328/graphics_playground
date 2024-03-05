#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cglm/cglm.h>
#include <GL/gl.h>
#include "glfw.h"

#include "heightmap.h"

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
            vertices[h][w][0] = spacing * w - w_offset;
            vertices[h][w][1] = h_offset - spacing * h;
            vertices[h][w][2] = heightmap_pixels[h][w] * scale;

            colors[h][w][0] = heightmap_pixels[h][w];
            colors[h][w][1] = heightmap_pixels[h][w];
            colors[h][w][2] = heightmap_pixels[h][w];
        }
    }
}

double start_time, current_time, delta_time;
vec3 camera_pos = {0.f, 0.f, 3.f};
vec3 camera_target;
vec3 camera_forward = {0.f, 0.f, -1.f};
vec3 camera_right;
vec3 camera_up;
float fov = 60.f;
mat4 view;
mat4 proj;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

/* mouse information */
bool firstmouse = true;
float yaw, pitch;
double lastx, lasty;

#define CAMERA_SNAP_POS GLM_VEC3_ZERO
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        firstmouse = true;
        /* points camera to snap position */
        glm_vec3_sub(CAMERA_SNAP_POS, camera_pos, camera_forward);
        glm_vec3_normalize(camera_forward);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float) yoffset * 2.5;
    if (fov < 10.f)
        fov = 10.f;
    if (fov > 60.f)
        fov = 60.f;
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstmouse) {
        pitch = asinf(camera_forward[1]);
        yaw = asinf(camera_forward[0] / cosf(pitch));
        lastx = xpos; lasty = ypos;
        firstmouse = false;
    }
    
    float xoffset = lastx - xpos;
    float yoffset = ypos - lasty; 
    lastx = xpos;
    lasty = ypos;

    float sensitivity = 0.25f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset * M_PI / 180.f;
    pitch += yoffset * M_PI / 180.f;

    const float one_deg = M_PI / 180.f;
    const float limit = M_PI / 4 - one_deg; /* 89 deg in rad */
    if(pitch > limit)
        pitch = limit;
    if(pitch < -limit)
        pitch = -limit;

    vec3 dir;
    dir[0] = sinf(yaw) * cosf(pitch);
    dir[1] = sinf(pitch);
    dir[2] = -cosf(yaw) * cosf(pitch);
    glm_vec3_normalize_to(dir, camera_forward);
}

void process_input() {
    vec3 temp = GLM_VEC3_ZERO_INIT;
    const float camera_speed = 4.f * delta_time;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm_vec3_scale(camera_forward, camera_speed, temp);
        glm_vec3_add(camera_pos, temp, camera_pos);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_scale(camera_forward, camera_speed, temp);
        glm_vec3_sub(camera_pos, temp, camera_pos);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm_vec3_scale(camera_right, camera_speed, temp);
        glm_vec3_add(camera_pos, temp, camera_pos);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm_vec3_scale(camera_right, camera_speed, temp);
        glm_vec3_sub(camera_pos, temp, camera_pos);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        glm_vec3_scale(camera_up, camera_speed, temp);
        glm_vec3_add(camera_pos, temp, camera_pos);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        glm_vec3_scale(camera_up, camera_speed, temp);
        glm_vec3_sub(camera_pos, temp, camera_pos);
    }
}

int main(void)
{
    uint32_t frames = 0;

    /* init glfw window */
    if (glfw_init() == -1) 
        return -1;

    glfw_set_key_callback(key_callback);
    glfw_set_mouse_button_callback(mouse_button_callback);
    glfw_set_scroll_callback(scroll_callback);
    glfw_set_cursor_pos_callback(cursor_position_callback);

    /* set clear color */
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glEnable(GL_DEPTH_TEST);

    /* Setup vertex and color arrays */
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &vertices[0][0][0]);
    glColorPointer(3, GL_UNSIGNED_BYTE, 0, &colors[0][0][0]);

    /* clears vertices memory */
    gen_vertices(HEIGHTMAP_HEIGHT, HEIGHTMAP_WIDTH, 0.01f, 0.001f);
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

        /* Camera updates */        
        glm_vec3_cross(camera_forward, GLM_YUP, camera_right);
        glm_vec3_normalize(camera_right);

        glm_vec3_cross(camera_forward, camera_right, camera_up);
        glm_vec3_normalize(camera_up);

        /* update camera position and view matrix */
        process_input();
        glm_vec3_add(camera_pos, camera_forward, camera_target);
        glm_lookat(camera_pos, camera_target, camera_up, view);

        /* Update projection matrix to fit window */
        glMatrixMode(GL_PROJECTION);
        glm_perspective(fov * M_PI / 180.f, width / (float) height, 0.1f, 100.f, proj);
        glLoadMatrixf(&proj[0][0]);

        /* Update model matrix to rotate */
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(&view[0][0]);

        /* clear frame and draw new elements */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
