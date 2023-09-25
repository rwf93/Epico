#include "tools.h"
#include "pipeline.h"

using namespace render;

RenderPipelineConstructor::RenderPipelineConstructor(VkDevice device, VkPipelineLayout pipeline_layout, VkPipelineCache pipeline_cache, VkRenderPass render_pass) {
	this->input_assembly		= {};
	this->viewport_state		= {};
	this->rasterizer			= {};
	this->multisampling			= {};
	this->color_attachments		= {};
	this->color_blending		= {};
	this->dynamic_states		= {};
	this->shader_stages			= {};
	this->depth_stencil			= {};
	this->pipeline_info			= {};

	// set sTypes for all types... actual bruh
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	this->device = device;
	this->pipeline_cache = pipeline_cache;
	this->pipeline_layout = pipeline_layout;
	this->render_pass = render_pass;
}

RenderPipelineConstructor::~RenderPipelineConstructor() {

}

void RenderPipelineConstructor::add_shader(VkShaderStageFlagBits stage, VkShaderModule shader) {
	VkPipelineShaderStageCreateInfo shader_stage_info = {};
	shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_info.stage = stage;
	shader_stage_info.module = shader;
	shader_stage_info.pName = "main";

	shader_stages.push_back(shader_stage_info);
}

VkResult RenderPipelineConstructor::build(VkPipeline *pipeline) {
	color_blending.attachmentCount = static_cast<uint32_t>(color_attachments.size());
	color_blending.pAttachments = color_attachments.data();

	VkPipelineDynamicStateCreateInfo dynamic_info = {};
	dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
	dynamic_info.pDynamicStates = dynamic_states.data();

	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size());
	pipeline_info.pStages = shader_stages.data();
	pipeline_info.pVertexInputState = &input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.pDynamicState = &dynamic_info;
	pipeline_info.layout = pipeline_layout;
	pipeline_info.renderPass = render_pass;

	return vkCreateGraphicsPipelines(device, pipeline_cache, 1, &pipeline_info, nullptr, pipeline);
}
