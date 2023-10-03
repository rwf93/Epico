#pragma once

#include <vk_mem_alloc.h>

namespace render {

class Renderer;

class EImage {
public:
    VkImage &get_image() { return image; }
    VmaAllocation &get_allocation() { return allocation; }
public:
    Renderer *context = nullptr;
    VkImage image = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;
};

}