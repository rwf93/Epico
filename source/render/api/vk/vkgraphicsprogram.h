#pragma once

class VulkanDevice;
class VulkanGraphicsProgram: public RenderResource {
public:
	VulkanGraphicsProgram(VulkanDevice *vkdevice)
		: device(vkdevice) {}
	~VulkanGraphicsProgram() override { if(get_state() != ResourceState::UNREADY) fini(); }

	VK_TRACY_MEMORY_OVERLOADS;

	ResourceState get_state() override { return state; };
	VkPipeline get_pipeline() { return pipeline; }
	VkPipelineLayout get_layout() { return layout; }

	void init(VkGraphicsPipelineCreateInfo *pipeline_create_info);
	void fini() override;

private:
	VulkanDevice *device;
	ResourceState state = ResourceState::UNREADY;

	VkPipeline pipeline;
	VkPipelineLayout layout;
};