#pragma once

#include "base.h"

void transitionImageLayout(Renderer *renderer, u32 imageIndex,
                           VkImageLayout oldLayout, VkImageLayout newLayout,
                           VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask,
                           VkPipelineStageFlags2 srcStageMask,
                           VkPipelineStageFlags2 dstStageMask);
void recordCommandBuffer(Renderer *renderer, u32 imageIndex, double delta_t, Vec2 delta_mouse);
