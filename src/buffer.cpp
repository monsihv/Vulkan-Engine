#include "headers/base.h"
#include "headers/buffer.h"

#include <stdio.h>

uint32_t findMemoryType(vk::PhysicalDevice gpu, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    auto memoryProperties = gpu.getMemoryProperties();

    for (uint32_t i {}; i < memoryProperties.memoryTypeCount; ++i) {
        if (typeFilter & (1 << i) &&
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) return i;
    }

    fprintf(stderr, "failed to find suitable memory type!");
    exit(1);
}
