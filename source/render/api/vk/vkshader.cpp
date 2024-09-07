#include "vkdevice.h"
#include "vkshader.h"

void VulkanShader::init(
	VkShaderModuleCreateInfo *vkcreateinfo,
	VkPipelineShaderStageCreateInfo *vkstageinfo
) {
	this->stage_info = *vkstageinfo;
	VK_CHECK(vkCreateShaderModule(device->get_device(), vkcreateinfo, nullptr, &shader_module));

	stage_info.module = shader_module;
	state = ResourceState::READY;
}

void VulkanShader::fini() {
	vkDestroyShaderModule(device->get_device(), shader_module, nullptr);
	state = ResourceState::UNREADY;
}