#include "vkdevice.h"
#include "vkprogrammanager.h"
#include "vklayoutmanager.h"

VulkanProgramManager::VulkanProgramManager() {}

VulkanProgramManager::~VulkanProgramManager() {}

void VulkanProgramManager::init(FunctorQueue<> &queue, VulkanDevice *vkdevice, VulkanLayoutManager *vklayoutmanager) {
	this->device = vkdevice;

	graphics_builder.init(device, vklayoutmanager);

	queue.push([&]() { fini(); });
}

void VulkanProgramManager::fini() {
	for(auto &map: shaders) {
		if(auto shader = map.second) {
			if(shader->get_state() == ShaderState::READY)
				shader->fini();

			delete shader;
		}
	}

	graphics_builder.fini();
}

RenderGraphicProgramBuilder *VulkanProgramManager::create_graphic_shader() {
	ShaderHandle last_shader_handle = shaders.size() + 1;

	auto graphic_shader = new VulkanGraphicsProgram(device);
	shaders.insert(std::make_pair(last_shader_handle, graphic_shader));

	graphics_builder.clear(last_shader_handle, graphic_shader);

	return &graphics_builder;
}