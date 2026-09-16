#pragma once

#include "include.h"

class Renderer;

struct Buffer {
    vk::Buffer buffer = nullptr;
    vk::DeviceMemory memory = nullptr;
    void *map = nullptr;
};

uint32_t findMemoryType(vk::PhysicalDevice gpu, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
Buffer createBuffer(Renderer *renderer, const void *data, vk::DeviceSize size,
                    vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryFlags);
void destroyBuffer(Renderer *renderer, Buffer& buffer);
void copyBuffer(Renderer *renderer, vk::Buffer& src, vk::Buffer& dst,
                vk::DeviceSize size, vk::CommandBuffer& commandBuffer);
