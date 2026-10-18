#pragma once

#include "include.h"

typedef struct {
    VkImage image;
    VmaAllocation memory;
    VkImageView imageView;
} Image;
