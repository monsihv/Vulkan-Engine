#pragma once

#include "include.h"
#include "buffer.h"

typedef struct {
    Mat4 view;
    Mat4 proj;
} Camera;

typedef struct {
    Camera camera;
    Vec4 position;   // point, w = 1
    float yaw;
    float pitch;
    float moveSpeed;
    float mouseSensitivity;
} CameraState;
