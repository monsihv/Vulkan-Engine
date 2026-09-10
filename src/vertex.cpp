#include "headers/vertex.h"
#include "headers/buffer.h"
#include "headers/base.h"

void createVertexBuffer(Renderer *renderer) {
    vk::BufferCreateInfo bufferInfo {
        .size = sizeof(renderer->mesh[0]) * renderer->mesh.size(),
        .usage = vk::BufferUsageFlagBits::eVertexBuffer,
        .sharingMode = vk::SharingMode::eExclusive
    };

    renderer->triangleData.buffer = vk::raii::Buffer(renderer->device, bufferInfo);

    auto memoryRequirements = renderer->triangleData.buffer.getMemoryRequirements();

    vk::MemoryAllocateInfo memoryInfo {
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = findMemoryType(*renderer->GPU, memoryRequirements.memoryTypeBits,
                                          vk::MemoryPropertyFlagBits::eHostVisible |
                                          vk::MemoryPropertyFlagBits::eHostCoherent)
    };

    renderer->triangleData.memory = vk::raii::DeviceMemory(renderer->device, memoryInfo);

    renderer->triangleData.buffer.bindMemory(*renderer->triangleData.memory, 0);

    renderer->triangleData.map = renderer->triangleData.memory.mapMemory(0, bufferInfo.size);

    memcpy(renderer->triangleData.map, renderer->mesh.data(), bufferInfo.size);
}










