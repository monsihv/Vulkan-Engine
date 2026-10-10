#include "headers/base.h"
#include "headers/draw.h"

#include <assert.h>

void transitionImageLayout(Renderer *renderer, u32 imageIndex,
                           VkImageLayout oldLayout, VkImageLayout newLayout,
                           VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask,
                           VkPipelineStageFlags2 srcStageMask,
                           VkPipelineStageFlags2 dstStageMask) {

    VkImageMemoryBarrier2 barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = renderer->swapchainImages[imageIndex],
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    VkDependencyInfo dependencyInfo = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .dependencyFlags = 0,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };

    vkCmdPipelineBarrier2(renderer->commandBuffers[renderer->frameIndex], &dependencyInfo);
}

void recordCommandBuffer(Renderer *renderer, u32 imageIndex, double delta_t, Vec2 delta_mouse) {
    auto frameIndex = renderer->frameIndex;
    VkCommandBuffer commandBuffer = renderer->commandBuffers[frameIndex];

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    transitionImageLayout(renderer, imageIndex,
                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                          0, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                          VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                          VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

    VkClearValue clearColor = {.color = {{0.216f, 0.216f, 0.216f, 0.216f}}};
    VkRenderingAttachmentInfo attachmentInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = renderer->swapchainImageViews[imageIndex],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = clearColor
    };

    VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {.offset = {0, 0}, .extent = renderer->swapchainExtent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };

    vkCmdBeginRendering(commandBuffer, &renderingInfo);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->graphicsPipeline);

    VkViewport viewport = {0.0f, 0.0f, (float)renderer->width, (float)renderer->height, 0.0f, 1.0f};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, renderer->swapchainExtent};
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    updateCameraBuffer(renderer, frameIndex, delta_t, delta_mouse);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderer->graphicsPipelineLayout,
                            0, 1, &renderer->cameraBufferDescriptorSets[frameIndex],
                            0, NULL);

    vkCmdPushConstants(commandBuffer, renderer->graphicsPipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT,
                       0, sizeof(Mat4), &renderer->model);

    VkDeviceSize vertexOffset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &renderer->meshVertices.buffer, &vertexOffset);
    vkCmdBindIndexBuffer(commandBuffer, renderer->meshIndices.buffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(commandBuffer, renderer->indexCount, 1, 0, 0, 0);

    vkCmdEndRendering(commandBuffer);

    transitionImageLayout(renderer, imageIndex,
                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                          VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                          VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 0,
                          VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                          VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);

    VK_CHECK(vkEndCommandBuffer(commandBuffer));
}

void createDrawSyncPrimitives(Renderer *renderer) {
    VkSemaphoreCreateInfo readySemaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    VkSemaphoreCreateInfo completeSemaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    VkFenceCreateInfo drawFenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    VkDevice device = renderer->device;
    auto size = renderer->swapchainImageCount;
    for (u32 i = 0; i < size; ++i) {
        if (i < MaxFramesInFlight) {
            VK_CHECK(vkCreateSemaphore(device, &readySemaphoreInfo, NULL, &renderer->renderReadySemaphores[i]));
            VK_CHECK(vkCreateFence(device, &drawFenceInfo, NULL, &renderer->drawFences[i]));
        }

        VK_CHECK(vkCreateSemaphore(device, &completeSemaphoreInfo, NULL, &renderer->renderCompleteSemaphores[i]));
    }
}

void drawFrame(Renderer *renderer) {
    auto& frameIndex = renderer->frameIndex;
    VkDevice device = renderer->device;

    auto fenceResult = vkWaitForFences(device, 1, &renderer->drawFences[frameIndex], VK_TRUE, UINT64_MAX);
    if (fenceResult != VK_SUCCESS) {
        fprintf(stderr, "failed to wait for fence\n");
        exit(1);
    }

    auto delta_t = glfwGetTime() - renderer->timer;
    renderer->timer = glfwGetTime();

    double x, y;
    glfwGetCursorPos(renderer->window, &x, &y);
    Vec2 pos = v2((float)x, (float)y);

    auto delta_mouse = pos - renderer->previousCursorPos;

    auto& mousePos = renderer->previousCursorPos;
    glfwGetCursorPos(renderer->window, &x, &y);
    mousePos.X = (float)x;
    mousePos.Y = (float)y;

    u32 imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(device, renderer->swapchain, UINT64_MAX,
                                            renderer->renderReadySemaphores[frameIndex],
                                            VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR) {
        recreateSwapchain(renderer);

        return;
    }
    else if (result != VK_SUCCESS) {
        fprintf(stderr, "failed to acquire swapchain image\n");
        exit(1);
    }

    VK_CHECK(vkResetFences(device, 1, &renderer->drawFences[frameIndex]));

    VK_CHECK(vkResetCommandBuffer(renderer->commandBuffers[frameIndex], 0));
    recordCommandBuffer(renderer, imageIndex, delta_t, delta_mouse);

    VkPipelineStageFlags waitDestinationStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    const VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderer->renderReadySemaphores[frameIndex],
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &renderer->commandBuffers[frameIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &renderer->renderCompleteSemaphores[imageIndex]
    };

    VK_CHECK(vkQueueSubmit(renderer->graphicsQueue, 1, &submitInfo, renderer->drawFences[frameIndex]));

    const VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderer->renderCompleteSemaphores[imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &renderer->swapchain,
        .pImageIndices = &imageIndex
    };

    VkResult presentResult = vkQueuePresentKHR(renderer->graphicsQueue, &presentInfo);

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR ||
        presentResult == VK_SUBOPTIMAL_KHR ||
        renderer->framebufferResized) {

        renderer->framebufferResized = false;
        recreateSwapchain(renderer);
    }
    else {
        assert(presentResult == VK_SUCCESS);
    }

    frameIndex = (frameIndex + 1) % MaxFramesInFlight;
}

void destroyDrawSyncPrimitives(Renderer *renderer) {
    VkDevice device = renderer->device;
    auto size = renderer->swapchainImageCount;
    for (u32 i = 0; i < size; ++i) {
        if (i < MaxFramesInFlight) {
            vkDestroySemaphore(device, renderer->renderReadySemaphores[i], NULL);
            vkDestroyFence(device, renderer->drawFences[i], NULL);
        }

        vkDestroySemaphore(device, renderer->renderCompleteSemaphores[i], NULL);
    }
}
