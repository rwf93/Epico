#include "info.h"

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

VkBufferCreateInfo info::buffer_create_info(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags flags) {
	VkBufferCreateInfo buffer_info = {};

	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.flags = flags;

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

VkDescriptorSetAllocateInfo info::descriptor_set_allocate_info(std::vector<VkDescriptorSetLayout> &allocate_info, VkDescriptorPool descriptor_pool) {
	VkDescriptorSetAllocateInfo set_allocate_info = {};

	set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	set_allocate_info.descriptorSetCount = static_cast<uint32_t>(allocate_info.size());
	set_allocate_info.pSetLayouts = allocate_info.data();
	set_allocate_info.descriptorPool = descriptor_pool;

	return set_allocate_info;
}

VkDescriptorSetLayoutCreateInfo info::descriptor_set_layout_info(std::vector<VkDescriptorSetLayoutBinding> &layout_info, VkDescriptorSetLayoutCreateFlags flags) {
	VkDescriptorSetLayoutCreateInfo set_layout_info = {};

	set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set_layout_info.bindingCount = static_cast<uint32_t>(layout_info.size());
	set_layout_info.pBindings = layout_info.data();
	set_layout_info.flags = flags;


	return set_layout_info;
}

VkViewport info::viewport(float width, float height, float x, float y, float min_depth, float max_depth) {
	VkViewport viewport = {};

	viewport.width = width;
	viewport.height = height;
	viewport.x = x;
	viewport.y = y;
	viewport.minDepth = min_depth;
	viewport.maxDepth = max_depth;

	return viewport;
}

VkRect2D info::rect2d(VkOffset2D offset, VkExtent2D extent) {
	return { offset, extent };
}

VkPipelineLayoutCreateInfo info::pipeline_layout_info(std::vector<VkDescriptorSetLayout> &descriptor_layouts) {
	VkPipelineLayoutCreateInfo pipeline_layout_info = {};

	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_layouts.size());
	pipeline_layout_info.pSetLayouts = descriptor_layouts.data();

	return pipeline_layout_info;
}

VkPipelineVertexInputStateCreateInfo info::input_vertex_info(
	std::vector<VkVertexInputBindingDescription> &bindings,
	std::vector<VkVertexInputAttributeDescription> &attributes
) {
	VkPipelineVertexInputStateCreateInfo input_vertex_info = {};

	input_vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	input_vertex_info.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
	input_vertex_info.pVertexBindingDescriptions = bindings.data();
	input_vertex_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
	input_vertex_info.pVertexAttributeDescriptions = attributes.data();

	return input_vertex_info;
}

VkPipelineRenderingCreateInfoKHR info::rendering_create_info(std::vector<VkFormat> &color_attachment_formats, VkFormat depth_format, VkFormat stencil_format) {
	VkPipelineRenderingCreateInfoKHR rendering_create_info = {};

	rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	rendering_create_info.colorAttachmentCount = static_cast<uint32_t>(color_attachment_formats.size());
	rendering_create_info.pColorAttachmentFormats = color_attachment_formats.data();
	rendering_create_info.depthAttachmentFormat = depth_format;
	rendering_create_info.stencilAttachmentFormat = stencil_format;

	return rendering_create_info;
}

VkImageCreateInfo info::image_create_info(int width, int height) {
	VkImageCreateInfo image_create_info = {};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.extent.width = width;
	image_create_info.extent.height = height;
	image_create_info.extent.depth = 1; // what the fuck is this used for???

	return image_create_info;
}

VkImageCreateInfo info::image_create_info(VkExtent2D extent) {
	return image_create_info(extent.width, extent.height);
}