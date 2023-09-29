#include "buffer.h"
#include "renderer.h"

using namespace render;

VkResult EBuffer::allocate(VmaAllocator vma_allocator,
							VkBufferCreateInfo *buffer_info,
							VmaAllocationCreateInfo *create_info,
							VmaAllocationInfo *allocation_info
) {
	assert(vma_allocator != VK_NULL_HANDLE);
	this->allocator = vma_allocator;

	return vmaCreateBuffer(allocator, buffer_info, create_info, &buffer, &allocation, allocation_info);
}


void EBuffer::stage(Renderer *renderer, EBuffer *staging_buffer, VkDeviceSize size) {
	renderer->submit_command([&](VkCommandBuffer command) {
		VkBufferCopy staging_copy;
		staging_copy.dstOffset = 0;
		staging_copy.srcOffset = 0;
		staging_copy.size = size;

		vmaFlushAllocation(allocator, staging_buffer->allocation, 0, VK_WHOLE_SIZE);
		vkCmdCopyBuffer(command, staging_buffer->buffer, buffer, 1, &staging_copy);
	});
}

void EBuffer::destroy() {
	vmaDestroyBuffer(allocator, buffer, allocation);
}
