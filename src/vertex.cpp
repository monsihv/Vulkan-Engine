#include "headers/vertex.h"
#include "headers/buffer.h"
#include "headers/base.h"

#include <string.h>

void createVertexBuffer(Renderer *renderer) {
    vk::CommandBufferAllocateInfo commandBufferInfo {
        .commandPool = renderer->commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    auto commandBuffer = renderer->device.allocateCommandBuffers(commandBufferInfo).front();

    auto data = renderer->mesh.data();
    auto size = sizeof(renderer->mesh[0]) * renderer->mesh.size();

    auto stageUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    auto stageAllocationFlags = vmaHostAccessRandom | vmaKeepMapped;
    auto stageBuffer = createBuffer(renderer, data, size, stageUsageFlags, stageAllocationFlags, NULL);

    auto vertexUsageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    auto vertexAllocationFlags {0u};
    renderer->meshVertices = createBuffer(renderer, NULL, size, vertexUsageFlags, vertexAllocationFlags, NULL);

    copyBuffer(renderer, stageBuffer.buffer, renderer->meshVertices.buffer, size, commandBuffer);
    destroyBuffer(renderer, stageBuffer);

    renderer->device.free(renderer->commandPool, commandBuffer);
}

void createIndexBuffer(Renderer *renderer) {
    vk::CommandBufferAllocateInfo commandBufferInfo {
        .commandPool = renderer->commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    auto commandBuffer = renderer->device.allocateCommandBuffers(commandBufferInfo).front();

    auto data = renderer->indices.data();
    auto size = sizeof(renderer->indices[0]) * renderer->indices.size();

    auto stageUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    auto stageAllocationFlags = vmaHostAccessRandom | vmaKeepMapped;
    auto stageBuffer = createBuffer(renderer, data, size, stageUsageFlags, stageAllocationFlags, NULL);

    auto indexUsageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    auto indexAllocationFlags {0u};
    renderer->meshIndices = createBuffer(renderer, NULL, size, indexUsageFlags, indexAllocationFlags, NULL);

    copyBuffer(renderer, stageBuffer.buffer, renderer->meshIndices.buffer, size, commandBuffer);
    destroyBuffer(renderer, stageBuffer);

    renderer->device.free(renderer->commandPool, commandBuffer);
}

void freeMeshBuffers(Renderer *renderer) {
    destroyBuffer(renderer, renderer->meshVertices);
    destroyBuffer(renderer, renderer->meshIndices);
}






