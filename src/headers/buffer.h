#pragma once

#include "include.h"

#define vmaHostAccessWriteNoRead VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
#define vmaHostAccessRandom VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
#define vmaKeepMapped VMA_ALLOCATION_CREATE_MAPPED_BIT

class Renderer;

struct Buffer {
    vk::Buffer buffer = nullptr;
    VmaAllocation memory;
    void *map = nullptr;
};

Buffer createBuffer(Renderer *renderer, const void *data, vk::DeviceSize size,
                    vk::BufferUsageFlags usageFlags, VmaAllocationCreateFlags allocationFlags,
                    VmaPool pool);
void destroyBuffer(Renderer *renderer, Buffer& buffer);
void copyBuffer(Renderer *renderer, vk::Buffer& src, vk::Buffer& dst,
                vk::DeviceSize size, vk::CommandBuffer& commandBuffer);
