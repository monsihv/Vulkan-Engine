#pragma once

#include "types.h"
#include "arena.h"

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define VK_CHECK(call)                                                      \
    do {                                                                    \
        VkResult vkCheckResult_ = (call);                                   \
        if (vkCheckResult_ != VK_SUCCESS) {                                 \
            fprintf(stderr, "%s:%d: %s failed with VkResult %d\n",          \
                    __FILE__, __LINE__, #call, (int)vkCheckResult_);        \
            exit(1);                                                        \
        }                                                                   \
    } while (0)
