#include "image.h"

#include "renderer.h"

using namespace render;

VkResult EImage::allocate(Renderer *renderer, VkImageCreateInfo *image_info, VmaAllocationCreateInfo *allocate_info) {
    assert(renderer != nullptr);
    this->context = renderer;

    return vmaCreateImage(context->get_allocator(), image_info, allocate_info, &image, &allocation, &info);
}

void EImage::destroy() {
    vmaDestroyImage(context->get_allocator(), image, allocation);
}

void load_image(Renderer *renderer, const char *path) {
    UNUSED(renderer);
    UNUSED(path);
}