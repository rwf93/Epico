#pragma once

class VulkanDevice;
class VulkanGraphicsProgram: public RenderProgram {
public:
	VulkanGraphicsProgram(VulkanDevice *vkdevice);
	~VulkanGraphicsProgram() override;

	ShaderState get_state() override { return state; };
	VkPipeline get_pipeline() { return pipeline; }
	VkPipelineLayout get_layout() { return layout; }

	void init(VkGraphicsPipelineCreateInfo *pipeline_create_info);
	void fini() override;

	VK_TRACY_MEMORY_OVERLOADS;

private:
	VulkanDevice *device;
	ShaderState state = ShaderState::UNREADY;

	VkPipeline pipeline;
	VkPipelineLayout layout;
};