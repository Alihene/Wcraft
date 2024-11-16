#include "camera.h"

#include <cglm/struct/cam.h>

void update_camera(Camera *camera, Window *window) {
    if(window->cursor_active) {
        return;
    }

    f32 aspect_ratio =
        (f32) window->dimensions.x
        / (f32) window->dimensions.y;

    f32 x = window->mouse.movement.x;
    f32 y = window->mouse.movement.y;

    f32 x_offset = x;
    f32 y_offset = y;

    f32 sens = 1.0f;
    x_offset *= sens;
    y_offset *= sens;

    camera->yaw += x_offset;
    camera->pitch += y_offset;

    if(camera->pitch > 89.0f) {
	    camera->pitch = 89.0f;
    } else if (camera->pitch < -89.0f) {
	    camera->pitch = -89.0f;
    }

    vec3s direction;
    direction.x = cosf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch));
    direction.y = sinf(glm_rad(camera->pitch));
    direction.z = sinf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch));
    camera->front = glms_normalize(direction);
    camera->up = (vec3s) {0.0f, 1.0f, 0.0f};

    camera->view = glms_lookat(camera->pos, glms_vec3_add(camera->pos, camera->front), camera->up);
    camera->proj = glms_perspective(glm_rad(CAMERA_FOV), aspect_ratio, 0.1f, 100.0f);
}