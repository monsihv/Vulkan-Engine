#pragma once

#include "include.h"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
};

constexpr u32 VertexAttributeCount = 2;

inline VkVertexInputBindingDescription getVertexBindingDescription() {
    VkVertexInputBindingDescription description = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    return description;
}

inline void getVertexAttributeDescriptions(VkVertexInputAttributeDescription out[VertexAttributeCount]) {
    out[0] = {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, pos)};
    out[1] = {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, color)};
}
