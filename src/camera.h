#ifndef _CAMERA_H
#define _CAMERA_H

#include <cglm/struct.h>

#include "util.h"
#include "window.h"

// Vertical view angle
#define CAMERA_FOV 80.0f

typedef struct {
    vec3s pos;
    
    vec3s up, front;

    mat4s view;
    mat4s proj;

    f32 pitch;
    f32 yaw;
} Camera;

void update_camera(Camera *camera, Window *window);

#endif