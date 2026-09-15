#include "headers/vertex.h"
#include "headers/buffer.h"
#include "headers/base.h"

void createVertexBuffer(Renderer *renderer) {
    vk::CommandBufferAllocateInfo commandBufferInfo {
        .commandPool = renderer->commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    auto commandBuffer = std::move(vk::raii::CommandBuffers(renderer->device, commandBufferInfo).front());

    auto data = renderer->mesh.data();
    auto size = sizeof(renderer->mesh[0]) * renderer->mesh.size();

    auto stageUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    auto stageMemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    auto stageBuffer = createBuffer(renderer, data, size, stageUsageFlags, stageMemoryFlags);

    auto vertexUsageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    auto vertexMemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
    renderer->meshVertices = createBuffer(renderer, NULL, size, vertexUsageFlags, vertexMemoryFlags);

    copyBuffer(renderer, stageBuffer.buffer, renderer->meshVertices.buffer, size, commandBuffer);
}

void createIndexBuffer(Renderer *renderer) {
    vk::CommandBufferAllocateInfo commandBufferInfo {
        .commandPool = renderer->commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    auto commandBuffer = std::move(vk::raii::CommandBuffers(renderer->device, commandBufferInfo).front());

    auto data = renderer->indices.data();
    auto size = sizeof(renderer->indices[0]) * renderer->indices.size();

    auto stageUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    auto stageMemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    auto stageBuffer = createBuffer(renderer, data, size, stageUsageFlags, stageMemoryFlags);

    auto indexUsageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    auto indexMemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
    renderer->meshIndices = createBuffer(renderer, NULL, size, indexUsageFlags, indexMemoryFlags);

    copyBuffer(renderer, stageBuffer.buffer, renderer->meshIndices.buffer, size, commandBuffer);
}








