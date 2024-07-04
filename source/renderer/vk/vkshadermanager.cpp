#include "vkdevice.h"
#include "vkshadermanager.h"

VulkanShaderManager::VulkanShaderManager() {}

VulkanShaderManager::~VulkanShaderManager() {}

void VulkanShaderManager::init(FunctorQueue<> &queue, VulkanDevice *vkdevice) {
	this->device = vkdevice;
	queue.push([&]() { fini(); });
}

void VulkanShaderManager::fini() {
	for(auto &map: shaders) {
		if(auto shader = map.second) {
			if(shader->get_state() == ShaderState::SHADER_READY)
				shader->fini();

			delete shader;
		}
	}
}

AbstractGraphicShaderBuilder *VulkanShaderManager::create_graphic_shader() {
	ShaderHandle last_shader_handle = advance_shader_handle();

	auto graphic_shader = new VulkanGraphicShader(device);
	shaders[last_shader_handle] = graphic_shader;

	graphics_builder.clear(last_shader_handle, graphic_shader, device, this);

	return &graphics_builder;
}