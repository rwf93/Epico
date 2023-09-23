#include "info.h"

using namespace render;

VkCommandPoolCreateInfo info::command_pool_create_info(uint32_t queue_family_index, VkCommandPoolCreateFlags flags)
{
	VkCommandPoolCreateInfo command_pool_info = {};

	command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.pNext = nullptr;
	command_pool_info.queueFamilyIndex = queue_family_index;
	command_pool_info.flags = flags;

	return command_pool_info;
}

VkCommandBufferAllocateInfo info::command_buffer_allocate_info(VkCommandPool command_pool, uint32_t count, VkCommandBufferLevel level) {
	VkCommandBufferAllocateInfo command_allocate_info = {};

	command_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_allocate_info.commandPool = command_pool;
	command_allocate_info.commandBufferCount = count;
	command_allocate_info.level = level;

	return command_allocate_info;
}

VkBufferCreateInfo info::buffer_create_info(VkDeviceSize size, VkBufferUsageFlags usage) {
	VkBufferCreateInfo buffer_info = {};

	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = usage;

	return buffer_info;
}

VmaAllocationCreateInfo info::allocation_create_info(VmaAllocationCreateFlags flags, VkMemoryPropertyFlags preferred_flags, VmaMemoryUsage usage, float priority) {
	VmaAllocationCreateInfo allocation_create_info = {};

	allocation_create_info.flags = flags;
	allocation_create_info.preferredFlags = preferred_flags;
	allocation_create_info.usage = usage;
	allocation_create_info.priority = priority;

	return allocation_create_info;
}

VkDescriptorSetLayoutBinding info::descriptor_set_layout_binding(VkDescriptorType type, VkShaderStageFlags stage_flags, uint32_t binding, uint32_t count) {
	VkDescriptorSetLayoutBinding set_layout_binding = {};

	set_layout_binding.descriptorType = type;
	set_layout_binding.stageFlags = stage_flags;
	set_layout_binding.binding = binding;
	set_layout_binding.descriptorCount = count;

	return set_layout_binding;
}

VkViewport info::viewport(float width, float height, float x, float y, float min_depth, float max_depth) {
	VkViewport viewport = {};
	viewport.width = width;
	viewport.height = height;
	viewport.x = x;
	viewport.y = y;
	viewport.minDepth = min_depth;
	viewport.minDepth = max_depth;

	return viewport;
}

VkRect2D info::rect2d(VkOffset2D offset, VkExtent2D extent) {
	return { offset, extent };
}