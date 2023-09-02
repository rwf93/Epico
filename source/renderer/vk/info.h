#pragma once

#include <vk_mem_alloc.h>

namespace render::info {

VkCommandPoolCreateInfo command_pool_create_info(uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0);
VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool command_pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

VkBufferCreateInfo buffer_create_info(VkDeviceSize size, VkBufferUsageFlags usage);
VmaAllocationCreateInfo allocation_create_info(VmaAllocationCreateFlags flags, VkMemoryPropertyFlags preferred_flags = 0, VmaMemoryUsage usage = VMA_MEMORY_USAGE_AUTO, float priority = 1.0f);

VkDescriptorSetLayoutBinding descriptor_set_layout_binding(VkDescriptorType type, VkShaderStageFlags stage_flags, uint32_t binding, uint32_t count = 1);

}