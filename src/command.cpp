#include "headers/base.h"

void createCommandPool(Renderer *renderer) {
    vk::CommandPoolCreateInfo commandPoolInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = renderer->graphicsQueueIndex
    };

    renderer->commandPool = renderer->device.createCommandPool(commandPoolInfo);
}

void allocateCommandBuffer(Renderer *renderer) {
    vk::CommandBufferAllocateInfo commandBufferInfo {
        .commandPool = renderer->commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = MaxFramesInFlight
    };

    renderer->commandBuffers = renderer->device.allocateCommandBuffers(commandBufferInfo);
}

void freeCommandBuffers(Renderer *renderer) {
    for (auto& commandBuffer : renderer->commandBuffers) {
        renderer->device.free(renderer->commandPool, commandBuffer);
    }
}
