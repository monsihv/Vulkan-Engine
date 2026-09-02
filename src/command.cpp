#include "headers/base.h"

void createCommandPool(Renderer *renderer) {
    vk::CommandPoolCreateInfo commandPoolInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = renderer->graphicsQueueIndex
    };

    renderer->commandPool = vk::raii::CommandPool(renderer->device, commandPoolInfo);
}

void allocateCommandBuffer(Renderer *renderer) {
    vk::CommandBufferAllocateInfo commandBufferInfo {
        .commandPool = renderer->commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = MaxFramesInFlight
    };

    renderer->commandBuffers = std::move(vk::raii::CommandBuffers(renderer->device, commandBufferInfo));
}
