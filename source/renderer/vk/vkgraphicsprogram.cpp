#include "vkdevice.h"
#include "vkgraphicsprogram.h"

VulkanGraphicsProgram::VulkanGraphicsProgram(VulkanDevice *vkdevice) {
	this->device = vkdevice;
}

VulkanGraphicsProgram::~VulkanGraphicsProgram() {}

void VulkanGraphicsProgram::init(VkGraphicsPipelineCreateInfo *pipeline_create_info) {
	layout = pipeline_create_info->layout; // Again, cheesing it here.
	VK_CHECK(vkCreateGraphicsPipelines(device->get_device(), VK_NULL_HANDLE, 1, pipeline_create_info, nullptr, &pipeline));
	state = ShaderState::READY;
}

void VulkanGraphicsProgram::fini() {
	vkDestroyPipeline(device->get_device(), pipeline, nullptr);
	state = ShaderState::READY;
}
