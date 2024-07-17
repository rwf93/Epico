#include "vkdevice.h"
#include "vkshadermanager.h"
#include "vklayoutmanager.h"

VulkanShaderManager::VulkanShaderManager() {}

VulkanShaderManager::~VulkanShaderManager() {}

void VulkanShaderManager::init(FunctorQueue<> &queue, VulkanDevice *vkdevice, VulkanLayoutManager *vklayoutmanager) {
	this->device = vkdevice;

	graphics_builder.init(device, vklayoutmanager);

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

	graphics_builder.fini();
}

AbstractGraphicShaderBuilder *VulkanShaderManager::create_graphic_shader() {
	ShaderHandle last_shader_handle = advance_handle();

	auto graphic_shader = new VulkanGraphicShader(device);
	shaders.insert(std::make_pair(last_shader_handle, graphic_shader));

	graphics_builder.clear(last_shader_handle, graphic_shader);

	return &graphics_builder;
}