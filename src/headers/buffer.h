#pragma once

#include "include.h"

#define vmaHostAccessWriteNoRead VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
#define vmaHostAccessRandom VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
#define vmaKeepMapped VMA_ALLOCATION_CREATE_MAPPED_BIT

typedef struct Renderer Renderer;

typedef struct {
    VkBuffer buffer;
    VmaAllocation memory;
    void *map;
} Buffer;

Buffer createBuffer(Renderer *renderer, const void *data, VkDeviceSize size,
                    VkBufferUsageFlags usageFlags, VmaAllocationCreateFlags allocationFlags,
                    VmaPool pool);
void destroyBuffer(Renderer *renderer, Buffer *buffer);
void copyBuffer(Renderer *renderer, VkBuffer src, VkBuffer dst,
                VkDeviceSize size, VkCommandBuffer commandBuffer);
