#include "vkdevice.h"
#include "vkgraphicsprogram.h"
#include "vkresourcemanager.h"
#include "vkgraphicsprogrambuilder.h"

void VulkanGraphicsProgramBuilder::init(VulkanDevice *vkdevice, VulkanResourceManager *vkresourcemanager) {
	this->device = vkdevice;
	this->resource_manager = vkresourcemanager;
	this->default_pipeline_layout = resource_manager->create_layout()->build();
}

void VulkanGraphicsProgramBuilder::clear(GraphicsProgramHandle program_handle) {
	this->handle = program_handle;

	bindings.clear();
	attributes.clear();
	shader_modules.clear();
	shader_stages.clear();
	color_states.clear();
	attachment_formats.clear();

	current_pipeline_layout = LayoutHandle::Invalid;

	assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	assembly_info.primitiveRestartEnable = VK_FALSE;

	viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.viewportCount = 1;
	viewport_info.scissorCount = 1;

	rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer_info.lineWidth = 1.0f;
	rasterizer_info.cullMode = VK_CULL_MODE_NONE;
	rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;

	multisampling_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling_info.sampleShadingEnable = VK_FALSE;
	multisampling_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling_info.alphaToOneEnable = VK_FALSE;
	multisampling_info.alphaToCoverageEnable = VK_FALSE;

	color_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_info.logicOpEnable = VK_FALSE;
	color_info.logicOp = VK_LOGIC_OP_COPY;

	stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	stencil_info.depthTestEnable = VK_FALSE;
	stencil_info.depthWriteEnable = VK_FALSE;
	stencil_info.depthCompareOp = VK_COMPARE_OP_NEVER;
	stencil_info.depthBoundsTestEnable = VK_FALSE;
	stencil_info.stencilTestEnable = VK_FALSE;
	stencil_info.front = {};
	stencil_info.back = {};
	stencil_info.minDepthBounds = 0.f;
	stencil_info.maxDepthBounds = 1.f;

	depth_format = VK_FORMAT_UNDEFINED;
}

GraphicsProgramHandle VulkanGraphicsProgramBuilder::build() {
	auto input_info = info::input_vertex_info(bindings, attributes);
	auto rendering_create_info = info::rendering_create_info(attachment_formats, depth_format);

	std::vector<VkDynamicState> dynamic_states = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamic_info = {};
	dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_info.pDynamicStates = dynamic_states.data();
	dynamic_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.pNext = &rendering_create_info;
	pipeline_info.pStages = shader_stages.data();
	pipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size());
	pipeline_info.pVertexInputState = &input_info;
	pipeline_info.pInputAssemblyState = &assembly_info;
	pipeline_info.pViewportState = &viewport_info;
	pipeline_info.pRasterizationState = &rasterizer_info;
	pipeline_info.pMultisampleState = &multisampling_info;
	pipeline_info.pColorBlendState = &color_info;
	pipeline_info.pDepthStencilState = &stencil_info;
	pipeline_info.pDynamicState = &dynamic_info;
	pipeline_info.layout = resource_manager
								->try_get_layout(current_pipeline_layout)
								.value_or(resource_manager->try_get_layout(default_pipeline_layout).value())
								->get_pipeline_layout();

	resource_manager->try_get_graphics_program(handle).value()->init(device, &pipeline_info);

	for(auto &module: shader_modules)
		vkDestroyShaderModule(device->get_device(), module, nullptr);

	return handle;
}

GraphicsProgramBuilder *VulkanGraphicsProgramBuilder::set_primitive(PrimitiveMode type) {
	assembly_info.topology = convert::convert_primitive_type(type);
	assembly_info.primitiveRestartEnable = VK_FALSE;

	return this;
};

GraphicsProgramBuilder *VulkanGraphicsProgramBuilder::set_polygon_mode(PolygonMode mode) {
	rasterizer_info.polygonMode = convert::convert_polygon_mode(mode);
	return this;
}

GraphicsProgramBuilder *VulkanGraphicsProgramBuilder::set_depth_format(ImageFormat format) {
	depth_format = convert::convert_image_format(format);
	return this;
}

GraphicsProgramBuilder *VulkanGraphicsProgramBuilder::set_depth_test(
	bool test_enable,
	bool write_enable,
	CompareOp compare
) {
	stencil_info.depthTestEnable = test_enable;
	stencil_info.depthWriteEnable = write_enable;
	stencil_info.depthCompareOp = convert::convert_compare_op(compare);
	rasterizer_info.depthBiasEnable = VK_TRUE;
	rasterizer_info.depthBiasConstantFactor = 4.0f;
	rasterizer_info.depthBiasSlopeFactor = 1.5f;

	return this;
}

GraphicsProgramBuilder *VulkanGraphicsProgramBuilder::add_binding(
	uint32_t size,
	BindingRate rate,
	uint32_t binding
) {
	VkVertexInputBindingDescription binding_description = {};
	binding_description.binding = binding;
	binding_description.stride = size;
	binding_description.inputRate = convert::convert_binding_rate(rate);

	bindings.push_back(binding_description);

	return this;
}

GraphicsProgramBuilder *VulkanGraphicsProgramBuilder::set_cull_face(CullFace face) {
	rasterizer_info.cullMode = convert::convert_cull_type(face);
	return this;
}

GraphicsProgramBuilder *VulkanGraphicsProgramBuilder::set_front_face(FrontFace face) {
	rasterizer_info.frontFace = convert::convert_face_type(face);
	return this;
}

GraphicsProgramBuilder *VulkanGraphicsProgramBuilder::add_attribute(
		uint32_t offset,
		AttributeType type,
		uint32_t binding
) {
	VkVertexInputAttributeDescription attribute_description = {};
	attribute_description.location = static_cast<uint32_t>(attributes.size());
	attribute_description.binding = binding;
	attribute_description.offset = offset;
	attribute_description.format = convert::convert_attribute_format(type);
	attributes.push_back(attribute_description);

	return this;
}

GraphicsProgramBuilder *VulkanGraphicsProgramBuilder::add_stage(
	ShaderStage stage,
	const char *data,
	size_t size
) {
	VkShaderModuleCreateInfo shader_create_info = {};
	shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_create_info.pCode = reinterpret_cast<const uint32_t*>(data);
	shader_create_info.codeSize = size;

	VkShaderModule shader_module = VK_NULL_HANDLE;
	VK_CHECK(vkCreateShaderModule(device->get_device(), &shader_create_info, nullptr, &shader_module));
	shader_modules.push_back(shader_module);

	uint64_t stage_bits = 0;

	if(stage & ShaderStage::VERTEX)
		stage_bits |= VK_SHADER_STAGE_VERTEX_BIT;

	if(stage & ShaderStage::FRAGMENT)
		stage_bits |= VK_SHADER_STAGE_FRAGMENT_BIT;

	VkPipelineShaderStageCreateInfo shader_stage_info = {};
	shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_info.module = shader_module;
	shader_stage_info.stage = static_cast<VkShaderStageFlagBits>(stage_bits);
	shader_stage_info.pName = "main";

	shader_stages.push_back(shader_stage_info);

	return this;
}

GraphicsProgramBuilder *VulkanGraphicsProgramBuilder::set_layout(LayoutHandle layout_handle) {
	current_pipeline_layout = layout_handle;
	return this;
}

GraphicsProgramBuilder *VulkanGraphicsProgramBuilder::add_attachment(ImageFormat format) {
	VkPipelineColorBlendAttachmentState color_blend_state = {};
	color_blend_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
								VK_COLOR_COMPONENT_G_BIT |
								VK_COLOR_COMPONENT_B_BIT |
								VK_COLOR_COMPONENT_A_BIT;
	color_blend_state.blendEnable = VK_FALSE;
	color_states.push_back(color_blend_state);

	attachment_formats.push_back(convert::convert_image_format(format));

	// Update color_info
	color_info.attachmentCount = static_cast<uint32_t>(color_states.size());
	color_info.pAttachments = color_states.data();

	return this;
}