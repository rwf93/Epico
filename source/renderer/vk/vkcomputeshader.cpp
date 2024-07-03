#include "vkcomputeshader.h"

VulkanComputeShader::VulkanComputeShader(ShaderHandle shader_handle, VulkanDevice *vkdevice) {
	this->handle = shader_handle;
	this->device = vkdevice;
}

VulkanComputeShader::~VulkanComputeShader() {}

ShaderHandle VulkanComputeShader::init() {
	state = ShaderState::SHADER_READY;
	return handle;
}

void VulkanComputeShader::fini() {
	state = ShaderState::SHADER_UNREADY;
}