#pragma once

#include "include.h"

struct Image {
    VkImage image;
    VmaAllocation memory;
    VkImageView imageView;
};
