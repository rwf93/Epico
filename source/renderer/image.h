#pragma once

#include <vk_mem_alloc.h>

namespace render {

class Renderer;

class EImage {
public:
    VkImage &get_image() { return image; }
    VmaAllocation &get_allocation() { return allocation; }
    VmaAllocationInfo &get_info() { return info; }

    VkResult allocate(Renderer *renderer, VkImageCreateInfo *image_info, VmaAllocationCreateInfo *allocate_info);
    void destroy();

    void load_image(Renderer *renderer, const char *path);
public:
    Renderer *context = nullptr;
    VkImage image = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocationInfo info = {};
};

}