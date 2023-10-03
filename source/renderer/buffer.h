#pragma once

#include <vk_mem_alloc.h>

//#include "renderer.h"

namespace render {

class Renderer;

class EBuffer {
public:
	VkBuffer &get_buffer() { return buffer; }
	VmaAllocation &get_allocation() { return allocation; }
	VmaAllocationInfo &get_info() { return info; }

	VkResult allocate(Renderer *renderer, VkBufferCreateInfo *buffer_info, VmaAllocationCreateInfo *create_info);
	void stage(EBuffer *staging_buffer, VkDeviceSize size);
	void destroy();

private:
	Renderer *context = nullptr;
	VkBuffer buffer = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;
	VmaAllocationInfo info = {};
};

}