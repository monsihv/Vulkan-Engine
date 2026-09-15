#pragma once

#include "include.h"

class Renderer;

struct Buffer {
    vk::raii::Buffer buffer = nullptr;
    vk::raii::DeviceMemory memory = nullptr;
    void *map = nullptr;
};

uint32_t findMemoryType(vk::PhysicalDevice gpu, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
Buffer createBuffer(Renderer *renderer, const void *data, vk::DeviceSize size,
                    vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryFlags);
void copyBuffer(Renderer *renderer, vk::raii::Buffer& src, vk::raii::Buffer& dst,
                vk::DeviceSize size, vk::raii::CommandBuffer& commandBuffer);
