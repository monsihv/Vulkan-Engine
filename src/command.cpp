#include "headers/base.h"

void createCommandPool(Renderer *renderer) {
    VkCommandPoolCreateInfo commandPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = renderer->graphicsQueueIndex
    };

    VK_CHECK(vkCreateCommandPool(renderer->device, &commandPoolInfo, NULL, &renderer->commandPool));
}

void allocateCommandBuffer(Renderer *renderer) {
    VkCommandBufferAllocateInfo commandBufferInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = renderer->commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MaxFramesInFlight
    };

    VK_CHECK(vkAllocateCommandBuffers(renderer->device, &commandBufferInfo, renderer->commandBuffers));
}

void freeCommandBuffers(Renderer *renderer) {
    vkFreeCommandBuffers(renderer->device, renderer->commandPool, MaxFramesInFlight, renderer->commandBuffers);
}
