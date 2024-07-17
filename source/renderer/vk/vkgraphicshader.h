#pragma once

class VulkanDevice;
class VulkanGraphicShader: public AbstractShader {
public:
	VulkanGraphicShader(VulkanDevice *vkdevice);
	~VulkanGraphicShader() override;

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