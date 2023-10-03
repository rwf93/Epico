#include "buffer.h"
#include "renderer.h"

using namespace render;

VkResult EBuffer::allocate(Renderer *renderer, VkBufferCreateInfo *buffer_info, VmaAllocationCreateInfo *create_info) {
	assert(renderer != VK_NULL_HANDLE);
	this->context = renderer;

	return vmaCreateBuffer(context->get_allocator(), buffer_info, create_info, &buffer, &allocation, &info);
}


void EBuffer::stage(EBuffer *staging_buffer, VkDeviceSize size) {
	context->submit_command([=, this](VkCommandBuffer command) {
		VkBufferCopy staging_copy;
		staging_copy.dstOffset = 0;
		staging_copy.srcOffset = 0;
		staging_copy.size = size;

		vmaFlushAllocation(context->get_allocator(), staging_buffer->get_allocation(), 0, VK_WHOLE_SIZE);
		vkCmdCopyBuffer(command, staging_buffer->get_buffer(), buffer, 1, &staging_copy);
	});
}

void EBuffer::destroy() {
	vmaDestroyBuffer(context->get_allocator(), buffer, allocation);
}
