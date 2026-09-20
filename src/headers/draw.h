#pragma once

#include "base.h"

void transitionImageLayout(Renderer *renderer, uint32_t imageIndex,
                           vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                           vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask,
                           vk::PipelineStageFlags2 srcStageMask,
                           vk::PipelineStageFlags2 dstStageMask);
void recordCommandBuffer(Renderer *renderer, uint32_t imageIndex, double delta_t, glm::vec2 delta_mouse);
