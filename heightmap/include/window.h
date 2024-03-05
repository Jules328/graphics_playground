#ifndef _GL_GLFW_H_
#define _GL_GLFW_H_

#include <math.h>
#include <stdio.h>

#include <GL/gl.h>
#include <cglm/cglm.h>
#include <GLFW/glfw3.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

GLFWwindow* window = NULL;

/* window properties*/
int width, height;
/* timing metrics */
double start_time, current_time, delta_time;
uint32_t frames = 0;
/* camera variables */
#define CAMERA_START_POS {0.f, 0.f, 3.f}
#define CAMERA_START_DIR {0.f, 1.f, 0.f}
vec3 camera_pos = CAMERA_START_POS;
vec3 camera_forward = CAMERA_START_DIR;
vec3 camera_target;
vec3 camera_right;
vec3 camera_up;
/* view matrices */
float fov = 60.f;
mat4 view;
mat4 proj;
/* mouse information */
bool camera_controled = false;
bool firstmouse = true;
float heading = 0.f, pitch = 0.f;
double lastx, lasty;

void error_callback( int error, const char *msg ) {
    printf("[%d] %s\n", error, msg);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    /* quit program */
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

#define CAMERA_SNAP_DIR (vec3)CAMERA_START_DIR
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        firstmouse = true; /* flag to reset cursor position state */
        /* points camera to snap position */
        // glm_vec3_sub(CAMERA_SNAP_POS, camera_pos, camera_forward);
        // glm_vec3_normalize(camera_forward);
        glm_vec3_copy(CAMERA_SNAP_DIR, camera_forward);
        /* sets heading and pitch */
        pitch = asinf(camera_forward[2]);
        heading = asinf(camera_forward[0] / cosf(pitch));
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            camera_controled = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
        } else if (action == GLFW_RELEASE) {
            camera_controled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float) yoffset * 2.5; /* decrease fov to zoom in */
    if (fov < 10.f)
        fov = 10.f;
    if (fov > 60.f)
        fov = 60.f;
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    printf("Time %0.3f: Cursor position: %f %f\n", glfwGetTime(), xpos, ypos);
    if (!camera_controled) {
        firstmouse = true;
        return;
    }

    if (firstmouse) {
        /* backs out pitch and heading from starting direction */
        lastx = xpos; lasty = ypos;
        firstmouse = false;
    }
    
    float xoffset = xpos - lastx;
    float yoffset = ypos - lasty; 
    lastx = xpos;
    lasty = ypos;

    float sensitivity = 0.25f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    heading += xoffset * M_PI / 180.f;
    pitch   -= yoffset * M_PI / 180.f;

    const float one_deg = M_PI / 180.f;
    const float limit = M_PI / 4 - one_deg; /* 89 deg in rad */
    if(pitch > limit)
        pitch = limit;
    if(pitch < -limit)
        pitch = -limit;

    vec3 dir;
    dir[0] = sinf(heading) * cosf(pitch);
    dir[1] = cosf(heading) * cosf(pitch);
    dir[2] = sinf(pitch);
    glm_vec3_normalize_to(dir, camera_forward);
}

void process_input() {
    vec3 temp = GLM_VEC3_ZERO_INIT;
    const float camera_speed = 4.f * delta_time;

    /* up */
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        glm_vec3_scale(camera_up, camera_speed, temp);
        glm_vec3_add(camera_pos, temp, camera_pos);
    }
    /* down */
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        glm_vec3_scale(camera_up, camera_speed, temp);
        glm_vec3_sub(camera_pos, temp, camera_pos);
    }
    /* left */
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm_vec3_scale(camera_right, camera_speed, temp);
        glm_vec3_sub(camera_pos, temp, camera_pos);
    }
    /* right */
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm_vec3_scale(camera_right, camera_speed, temp);
        glm_vec3_add(camera_pos, temp, camera_pos);
    }
    /* forward */
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm_vec3_scale(camera_forward, camera_speed, temp);
        glm_vec3_add(camera_pos, temp, camera_pos);
    }
    /* back */
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_scale(camera_forward, camera_speed, temp);
        glm_vec3_sub(camera_pos, temp, camera_pos);
    }
}

int window_init(void) {
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

    /* set window callbacks */
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    /* make the window's context current */
    glfwMakeContextCurrent(window);

    /* cap framerate at screen refresh */
    glfwSwapInterval(1);

    /* set clear color and enable depth testing */
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glEnable(GL_DEPTH_TEST);

    return 0;
}

void window_start(void) {
    start_time = glfwGetTime();
    current_time = start_time;
    delta_time = 1/60.f; /* guess at initial delta time */
}

void window_update(void) {
    /* poll for inputs */
    glfwPollEvents();

    /* clear frame */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* resize openGL viewport to window's size */
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    /* Camera updates */        
    glm_vec3_cross(camera_forward, GLM_ZUP, camera_right);
    glm_vec3_normalize(camera_right);

    glm_vec3_cross(camera_right, camera_forward, camera_up);
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
}

int window_draw_frame(void) {
    /* check GL errors */
    uint32_t error = glGetError();
    if (error != 0) {
        printf("OpenGL errorcode 0x%04x\n", error);
        return -1;
    }
    
    /* update window frame */
    glfwSwapBuffers(window);

    /* Update timing */
    delta_time = glfwGetTime() - current_time;
    current_time = glfwGetTime();
    if (++frames % 100 == 0) {
        printf("Average frames per second: %5.2f\n", frames / (current_time - start_time));
    }

    return 0;
}

int window_should_close(void) {
    return glfwWindowShouldClose(window);
}

void _get_window_size(int* width, int* height) {
    glfwGetFramebufferSize(window, width, height);
}

#endif /* _GL_GLFW_H_ */