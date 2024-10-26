#ifndef _CAMERA_H
#define _CAMERA_H

#include <cglm/struct.h>

#include "util.h"
#include "window.h"

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