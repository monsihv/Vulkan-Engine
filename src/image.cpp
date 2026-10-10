#include "headers/image.h"
#include "headers/base.h"

void destroyImage(Renderer *renderer, Image& image, bool hasImageView) {
    vmaDestroyImage(renderer->allocator, image.image, image.memory);
    if (hasImageView) vkDestroyImageView(renderer->device, image.imageView, NULL);
}
