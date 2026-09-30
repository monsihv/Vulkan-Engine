#pragma once

#include "include.h"
#include "buffer.h"

struct Camera {
    glm::mat4 view {};
    glm::mat4 proj {};
};

struct CameraState {
    Camera camera {};
    glm::vec3 position {0.0f, 0.0f, 3.0f};
    float yaw {};
    float pitch {};
    float moveSpeed = 1.0f;
    float mouseSensitivity = 0.0035f;
};
