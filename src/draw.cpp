#include "headers/base.h"
#include "headers/draw.h"

#include <stdio.h>
#include <stdlib.h>

void transitionImageLayout(Renderer *renderer, uint32_t imageIndex,
                           vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                           vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask,
                           vk::PipelineStageFlags2 srcStageMask,
                           vk::PipelineStageFlags2 dstStageMask) {

    vk::ImageMemoryBarrier2 barrier {
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = renderer->swapchainImages[imageIndex],
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    vk::DependencyInfo dependencyInfo {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };

    renderer->commandBuffers[renderer->frameIndex].pipelineBarrier2(dependencyInfo);
}

void recordCommandBuffer(Renderer *renderer, uint32_t imageIndex) {
    auto frameIndex = renderer->frameIndex;

    renderer->commandBuffers[frameIndex].begin(vk::CommandBufferBeginInfo{});

    transitionImageLayout(renderer, imageIndex,
                          vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 
                          {}, vk::AccessFlagBits2::eColorAttachmentWrite,
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput);

    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::RenderingAttachmentInfo attachmentInfo {
        .imageView = renderer->swapchainImageViews[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    vk::RenderingInfo renderingInfo {
        .renderArea = {.offset = {0, 0}, .extent = renderer->swapchainExtent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };

    renderer->commandBuffers[frameIndex].beginRendering(renderingInfo);

    renderer->commandBuffers[frameIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, renderer->graphicsPipeline);

    vk::Viewport viewport {0.0f, 0.0f, (float)renderer->width, (float)renderer->height, 0.0f, 1.0f};
    renderer->commandBuffers[frameIndex].setViewport(0, viewport);
    vk::Rect2D scissor {vk::Offset2D(0, 0), renderer->swapchainExtent};
    renderer->commandBuffers[frameIndex].setScissor(0, scissor);

    renderer->commandBuffers[frameIndex].bindVertexBuffers(0,
                                                          renderer->meshVertices.buffer, {0});
    renderer->commandBuffers[frameIndex].bindIndexBuffer(renderer->meshIndices.buffer, 0, vk::IndexType::eUint16);
    renderer->commandBuffers[frameIndex].drawIndexed(renderer->indices.size(), 1, 0, 0, 0);

    renderer->commandBuffers[frameIndex].endRendering();

    transitionImageLayout(renderer, imageIndex,
                          vk::ImageLayout::eColorAttachmentOptimal,
                          vk::ImageLayout::ePresentSrcKHR,
                          vk::AccessFlagBits2::eColorAttachmentWrite, {}, 
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                          vk::PipelineStageFlagBits2::eBottomOfPipe);

    renderer->commandBuffers[frameIndex].end();
}

void createDrawSyncPrimitives(Renderer *renderer) {
    vk::SemaphoreCreateInfo readySemaphoreInfo {};
    vk::SemaphoreCreateInfo completeSemaphoreInfo {};
    vk::FenceCreateInfo drawFenceInfo {
        .flags = vk::FenceCreateFlagBits::eSignaled
    };

    vk::Semaphore semaphore;
    vk::Fence fence;
    auto size = renderer->swapchainImages.size();
    for (size_t i {}; i < size; ++i) {
        if (i < MaxFramesInFlight) {
            semaphore = renderer->device.createSemaphore(readySemaphoreInfo);
            fence = renderer->device.createFence(drawFenceInfo);
            renderer->renderReadySemaphores.push_back(semaphore);
            renderer->drawFences.push_back(fence);
        }

        semaphore = renderer->device.createSemaphore(completeSemaphoreInfo);
        renderer->renderCompleteSemaphores.emplace_back(semaphore);
    }
}

void drawFrame(Renderer *renderer) {
    auto& frameIndex = renderer->frameIndex;

    auto fenceResult = renderer->device.waitForFences(renderer->drawFences[frameIndex], vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess) {
        fprintf(stderr, "failed to wait for fence\n");
        exit(1);
    }

    auto [result, imageIndex] = renderer->device.acquireNextImageKHR(renderer->swapchain, UINT64_MAX, 
                                                                     renderer->renderReadySemaphores[frameIndex],
                                                                     nullptr);

    if (result == vk::Result::eErrorOutOfDateKHR || 
        result == vk::Result::eSuboptimalKHR) {
        recreateSwapchain(renderer);

        return;
    }
    else if (result != vk::Result::eSuccess) {
        fprintf(stderr, "failed to acquire swapchain image\n");
        exit(1);
    }

    renderer->device.resetFences(renderer->drawFences[frameIndex]);

    renderer->commandBuffers[frameIndex].reset();
    recordCommandBuffer(renderer, imageIndex);

    vk::PipelineStageFlags waitDestinationStageMask {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    const vk::SubmitInfo submitInfo {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderer->renderReadySemaphores[frameIndex],
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &renderer->commandBuffers[frameIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &renderer->renderCompleteSemaphores[imageIndex]
    };

    renderer->graphicsQueue.submit(submitInfo, renderer->drawFences[frameIndex]);

    const vk::PresentInfoKHR presentInfo {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderer->renderCompleteSemaphores[imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &renderer->swapchain,
        .pImageIndices = &imageIndex
    };

    auto presentResult = renderer->graphicsQueue.presentKHR(presentInfo);

    if (presentResult == vk::Result::eErrorOutOfDateKHR || 
        presentResult == vk::Result::eSuboptimalKHR ||
        renderer->framebufferResized) {

        renderer->framebufferResized = false;
        recreateSwapchain(renderer);
    }
    else {
        assert(presentResult == vk::Result::eSuccess);
    }

    frameIndex = (frameIndex + 1) % MaxFramesInFlight;
}

void destroyDrawSyncPrimitives(Renderer *renderer) {
    auto size = renderer->swapchainImages.size();
    for (size_t i {}; i < size; ++i) {
        if (i < MaxFramesInFlight) {
            renderer->device.destroy(renderer->renderReadySemaphores[i]);
            renderer->device.destroy(renderer->drawFences[i]);
        }

        renderer->device.destroy(renderer->renderCompleteSemaphores[i]);
    }
}





