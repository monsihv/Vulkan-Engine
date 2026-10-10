#pragma once

#include "include.h"
#include "buffer.h"

struct Camera {
    Mat4 view;
    Mat4 proj;
};

struct CameraState {
    Camera camera;
    Vec3 position;
    float yaw;
    float pitch;
    float moveSpeed;
    float mouseSensitivity;
};
