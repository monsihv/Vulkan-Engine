#pragma once

#include "include.h"

// 32 bytes: pos at 0, color at 16. The vertex format reads xyz and ignores w.
typedef struct {
    Vec4 pos;     // w = 1
    Vec4 color;
} Vertex;

constexpr u32 VertexAttributeCount = 2;

static inline VkVertexInputBindingDescription getVertexBindingDescription(void) {
    VkVertexInputBindingDescription description = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    return description;
}

static inline void getVertexAttributeDescriptions(VkVertexInputAttributeDescription out[VertexAttributeCount]) {
    out[0] = (VkVertexInputAttributeDescription){.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, pos)};
    out[1] = (VkVertexInputAttributeDescription){.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, color)};
}
