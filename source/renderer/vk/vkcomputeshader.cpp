#include "vkcomputeshader.h"

VulkanComputeShader::VulkanComputeShader(VulkanDevice *vkdevice) {
	this->device = vkdevice;
}

VulkanComputeShader::~VulkanComputeShader() {}

void VulkanComputeShader::fini() {
	state = ShaderState::SHADER_UNREADY;
}