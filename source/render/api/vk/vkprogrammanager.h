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

	RenderGraphicProgramBuilder *create_graphic_program();

	std::optional<VulkanGraphicsProgram*> try_get_graphics_program(GraphicsProgramHandle handle) {
		if(!graphics_programs.contains(handle))
			return std::nullopt;

		auto resource = graphics_programs.at(handle);
		if(!resource)
			return std::nullopt;

		return dynamic_cast<VulkanGraphicsProgram*>(resource);
	}

private:
	VulkanDevice *device;
	VulkanGraphicsProgramBuilder graphics_builder;

	std::map<GraphicsProgramHandle, RenderProgram*> graphics_programs;
};