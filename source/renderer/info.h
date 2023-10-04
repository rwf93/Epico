#pragma once

#include <vk_mem_alloc.h>

namespace render::info {

VkCommandPoolCreateInfo command_pool_create_info(uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0);
VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool command_pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

VkBufferCreateInfo buffer_create_info(VkDeviceSize size, VkBufferUsageFlags usage);
VmaAllocationCreateInfo allocation_create_info(VmaAllocationCreateFlags flags, VkMemoryPropertyFlags preferred_flags = 0, VmaMemoryUsage usage = VMA_MEMORY_USAGE_AUTO, float priority = 1.0f);

VkDescriptorSetLayoutBinding descriptor_set_layout_binding(VkDescriptorType type, VkShaderStageFlags stage_flags, uint32_t binding, uint32_t count = 1);
VkDescriptorSetAllocateInfo descriptor_set_allocate_info(std::vector<VkDescriptorSetLayout> &allocate_info, VkDescriptorPool descriptor_pool);
VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info(std::vector<VkDescriptorSetLayoutBinding> &layout_info, VkDescriptorSetLayoutCreateFlags flags = 0);

VkViewport viewport(float width, float height, float x = 0, float y = 0, float min_depth = 0.0f, float max_depth = 1.0f);
VkRect2D rect2d(VkOffset2D offset, VkExtent2D extent); // mainly used for scissors, might be useless

VkPipelineLayoutCreateInfo pipeline_layout_info(std::vector<VkDescriptorSetLayout> &descriptor_layouts);

VkPipelineVertexInputStateCreateInfo input_vertex_info(
	std::vector<VkVertexInputBindingDescription> &bindings,
	std::vector<VkVertexInputAttributeDescription> &attributes
);

VkPipelineRenderingCreateInfoKHR rendering_create_info(std::vector<VkFormat> &color_attachment_formats, VkFormat depth_format, VkFormat stencil_format = VK_FORMAT_UNDEFINED);

}