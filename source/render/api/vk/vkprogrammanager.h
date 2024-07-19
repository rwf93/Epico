#pragma once

#include "vkgraphicsprogrambuilder.h"
#include "vkgraphicsprogram.h"

class VulkanDevice;
class VulkanProgramManager {
public:
	VulkanProgramManager();
	~VulkanProgramManager();

	void init(FunctorQueue<> &queue, VulkanDevice *vkdevice, VulkanLayoutManager *vklayoutmanager);
	void fini();

	RenderGraphicProgramBuilder *create_graphic_shader();

	VulkanGraphicsProgram *get_graphic_shader(ShaderHandle handle) {
		return get_shader<VulkanGraphicsProgram*>(handle);
	}

	template<typename T>
	T get_shader(ShaderHandle handle) {
		RenderProgram *shader = nullptr;
		if(!shaders.contains(handle))
			goto err;

		shader = shaders.at(handle);
		if(!shader)
			goto err;

		return dynamic_cast<T>(shader);
	err:
		return nullptr;
	}

private:
	VulkanDevice *device;
	VulkanGraphicsProgramBuilder graphics_builder;

	std::map<ShaderHandle, RenderProgram*> shaders;

	ShaderHandle current_shader_handle = 0;
};