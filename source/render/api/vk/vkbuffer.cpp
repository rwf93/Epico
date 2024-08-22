#include "vkdevice.h"
#include "vkcommandpool.h"
#include "vkbuffer.h"

void VulkanBuffer::init(
	VulkanDevice *vkdevice,
	VulkanCommandPool *vkcommandpool,
	VmaAllocator vkallocator,
	VkBufferCreateInfo *buffer_create_info,
	VmaAllocationCreateInfo *allocation_create_info
) {
	this->device = vkdevice;
	this->command_pool = vkcommandpool;
	this->allocator = vkallocator;
	this->create_info = *buffer_create_info;

	VK_CHECK(vmaCreateBuffer(
		allocator,
		buffer_create_info, allocation_create_info,
		&buffer, &allocation, &allocation_info
	));

	state = ResourceState::READY;
}

void VulkanBuffer::stage(
	VulkanBuffer *staging_buffer,
	VkDeviceSize size,
	VkDeviceSize src_offset,
	VkDeviceSize dst_offset
) {
	assert(get_state() == ResourceState::READY);

	command_pool->submit_command([&](VulkanCommand *command) {
		VkBufferCopy copy;
		copy.size = size;
		copy.srcOffset = src_offset;
		copy.dstOffset = dst_offset;

		std::vector<VkBufferCopy> copy_ranges = {
			copy
		};

		vmaFlushAllocation(allocator, staging_buffer->get_allocation(), 0, VK_WHOLE_SIZE);
		command->copy_buffer(staging_buffer->get_buffer(), buffer, copy_ranges);
	});
}

void VulkanBuffer::fini() {
	assert(get_state() == ResourceState::READY);

	vmaDestroyBuffer(allocator, buffer, allocation);
	state = ResourceState::UNREADY;
}