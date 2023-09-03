#pragma once

namespace render {
// 4l8r
class RenderPipelineConstructor {
public:
	RenderPipelineConstructor(VkDevice device, VkRenderPass render_pass, VkPipelineCache pipeline_cache);
	~RenderPipelineConstructor();

	void add_shader(VkShaderStageFlagBits stage, VkShaderModule shader);
	void build(VkPipeline *pipeline, VkPipelineLayout *pipeline_layout);

public:
	VkPipelineVertexInputStateCreateInfo input_info;
	VkPipelineInputAssemblyStateCreateInfo input_assembly;

	VkPipelineViewportStateCreateInfo viewport_state;

	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineMultisampleStateCreateInfo multisampling;

	std::vector<VkPipelineColorBlendAttachmentState> color_attachments;
	VkPipelineColorBlendStateCreateInfo color_blending;

	std::vector<VkDynamicState> dynamic_states;

	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

	VkPipelineDepthStencilStateCreateInfo depth_stencil;

	VkPipelineLayoutCreateInfo pipeline_layout_info;
	VkGraphicsPipelineCreateInfo pipeline_info;
private:
	VkDevice device;
	VkRenderPass pass;
	VkPipelineCache cache;
};

struct PipelinePair {
	VkPipeline pipeline;
	VkPipelineLayout layout;

	operator VkPipeline() { return pipeline; };
	operator VkPipelineLayout() { return layout; };
};

}