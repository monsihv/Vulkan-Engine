#include "headers/vertex.h"
#include "headers/buffer.h"
#include "headers/base.h"

void createVertexBuffer(Renderer *renderer) {
    VkCommandBufferAllocateInfo commandBufferInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = renderer->commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VkCommandBuffer commandBuffer;
    VK_CHECK(vkAllocateCommandBuffers(renderer->device, &commandBufferInfo, &commandBuffer));

    auto data = renderer->mesh;
    auto size = sizeof(renderer->mesh[0]) * renderer->meshCount;

    VkBufferUsageFlags stageUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    auto stageAllocationFlags = vmaHostAccessRandom | vmaKeepMapped;
    auto stageBuffer = createBuffer(renderer, data, size, stageUsageFlags, stageAllocationFlags, NULL);

    VkBufferUsageFlags vertexUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    auto vertexAllocationFlags {0u};
    renderer->meshVertices = createBuffer(renderer, NULL, size, vertexUsageFlags, vertexAllocationFlags, NULL);

    copyBuffer(renderer, stageBuffer.buffer, renderer->meshVertices.buffer, size, commandBuffer);
    destroyBuffer(renderer, stageBuffer);

    vkFreeCommandBuffers(renderer->device, renderer->commandPool, 1, &commandBuffer);
}

void createIndexBuffer(Renderer *renderer) {
    VkCommandBufferAllocateInfo commandBufferInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = renderer->commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VkCommandBuffer commandBuffer;
    VK_CHECK(vkAllocateCommandBuffers(renderer->device, &commandBufferInfo, &commandBuffer));

    auto data = renderer->indices;
    auto size = sizeof(renderer->indices[0]) * renderer->indexCount;

    VkBufferUsageFlags stageUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    auto stageAllocationFlags = vmaHostAccessRandom | vmaKeepMapped;
    auto stageBuffer = createBuffer(renderer, data, size, stageUsageFlags, stageAllocationFlags, NULL);

    VkBufferUsageFlags indexUsageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    auto indexAllocationFlags {0u};
    renderer->meshIndices = createBuffer(renderer, NULL, size, indexUsageFlags, indexAllocationFlags, NULL);

    copyBuffer(renderer, stageBuffer.buffer, renderer->meshIndices.buffer, size, commandBuffer);
    destroyBuffer(renderer, stageBuffer);

    vkFreeCommandBuffers(renderer->device, renderer->commandPool, 1, &commandBuffer);
}

void freeMeshBuffers(Renderer *renderer) {
    destroyBuffer(renderer, renderer->meshVertices);
    destroyBuffer(renderer, renderer->meshIndices);
}
