#pragma once

#include "include.h"

struct Image {
    vk::Image image;
    VmaAllocation memory;
    vk::ImageView imageView;
};
