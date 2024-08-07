#pragma once

class VulkanDevice;
class VulkanGraphicsProgram: public RenderResource {
public:
	VulkanGraphicsProgram() = default;
	~VulkanGraphicsProgram() override = default;

	ResourceState get_state() override { return state; };
	VkPipeline get_pipeline() { return pipeline; }
	VkPipelineLayout get_layout() { return layout; }

	void init(VulkanDevice *vkdevice, VkGraphicsPipelineCreateInfo *pipeline_create_info);
	void fini() override;

private:
	VulkanDevice *device;
	ResourceState state = ResourceState::UNREADY;

	VkPipeline pipeline;
	VkPipelineLayout layout;
};