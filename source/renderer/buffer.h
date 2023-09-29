#pragma once

#include <vk_mem_alloc.h>

//#include "renderer.h"

namespace render {

class Renderer;

struct EBuffer {
	VkBuffer buffer = VK_NULL_HANDLE;
	VmaAllocator allocator = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;

	operator VmaAllocation() { return allocation; }
	operator VkBuffer() { return buffer; };

	VkResult allocate(VmaAllocator vma_allocator, VkBufferCreateInfo *buffer_info, VmaAllocationCreateInfo *create_info, VmaAllocationInfo *allocation_info = nullptr);
	void stage(Renderer *renderer, EBuffer *staging_buffer, VkDeviceSize size);
	void destroy();
};

}