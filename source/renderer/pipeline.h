#pragma once

namespace render {
// 4l8r
class RenderPipelineConstructor {
public:
	RenderPipelineConstructor(VkDevice device, VkPipelineLayout pipeline_layout, VkPipelineCache pipeline_cache, VkRenderPass render_pass);
	~RenderPipelineConstructor();

	void add_shader(VkShaderStageFlagBits stage, VkShaderModule shader);
	VkResult build(VkPipeline *pipeline);

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

	VkGraphicsPipelineCreateInfo pipeline_info;
private:
	VkDevice device;
	VkPipelineCache pipeline_cache;
	VkPipelineLayout pipeline_layout;
	VkRenderPass render_pass;
};

}