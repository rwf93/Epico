#include "vkdevice.h"
#include "vkgraphicshader.h"
#include "vkgraphicshaderbuilder.h"
#include "vklayoutmanager.h"

VulkanGraphicShaderBuilder::VulkanGraphicShaderBuilder() {}
VulkanGraphicShaderBuilder::~VulkanGraphicShaderBuilder() {}

void VulkanGraphicShaderBuilder::init(VulkanDevice *vkdevice, VulkanLayoutManager *vklayoutmanager) {
	this->device = vkdevice;
	this->layout_manager = vklayoutmanager;
}

void VulkanGraphicShaderBuilder::clear(
	ShaderHandle shader_handle,
	VulkanGraphicShader *vkshader
) {
	this->handle = shader_handle;
	this->shader = vkshader;

	bindings.clear();
	attributes.clear();
	shader_modules.clear();
	shader_stages.clear();
	color_states.clear();
	attachment_formats.clear();
	descriptor_layouts.clear();

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

ShaderHandle VulkanGraphicShaderBuilder::build() {
	VkPipelineLayout pipeline_layout;
	auto pipeline_layout_info = info::pipeline_layout_info(descriptor_layouts);
	VK_CHECK(vkCreatePipelineLayout(device->get_device(), &pipeline_layout_info, nullptr, &pipeline_layout));

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
	pipeline_info.layout = pipeline_layout;

	shader->init(&pipeline_info);

	for(auto &module: shader_modules)
		vkDestroyShaderModule(device->get_device(), module, nullptr);

	return handle;
}

void VulkanGraphicShaderBuilder::fini() {
	for(auto &pipeline_layout: pipeline_layouts)
		vkDestroyPipelineLayout(device->get_device(), pipeline_layout, nullptr);
}

AbstractGraphicShaderBuilder *VulkanGraphicShaderBuilder::set_primitive(ShaderPrimitive type) {
	assembly_info.topology = convert::convert_primitive_type(type);
	assembly_info.primitiveRestartEnable = VK_FALSE;

	return this;
};

AbstractGraphicShaderBuilder *VulkanGraphicShaderBuilder::set_polygon_mode(ShaderPolygonMode mode) {
	rasterizer_info.polygonMode = convert::convert_polygon_mode(mode);
	return this;
}

AbstractGraphicShaderBuilder *VulkanGraphicShaderBuilder::set_depth_format(ImageFormat format) {
	depth_format = convert::convert_image_format(format);
	return this;
}

AbstractGraphicShaderBuilder *VulkanGraphicShaderBuilder::set_depth_test(bool write_enable, ShaderCompareOp compare) {
	stencil_info.depthTestEnable = VK_TRUE;
	stencil_info.depthWriteEnable = write_enable;
	stencil_info.depthCompareOp = convert::convert_compare_op(compare);
	stencil_info.depthBoundsTestEnable = VK_FALSE;
	stencil_info.stencilTestEnable = VK_FALSE;
	stencil_info.front = {};
	stencil_info.back = {};
	stencil_info.minDepthBounds = 0.f;
	stencil_info.maxDepthBounds = 1.f;

	return this;
}

AbstractGraphicShaderBuilder *VulkanGraphicShaderBuilder::add_binding(
	uint32_t binding,
	uint32_t size,
	BindingRate rate
) {
	VkVertexInputBindingDescription binding_description = {};
	binding_description.binding = binding;
	binding_description.stride = size;
	binding_description.inputRate = convert::convert_binding_rate(rate);

	bindings.push_back(binding_description);

	return this;
}

AbstractGraphicShaderBuilder *VulkanGraphicShaderBuilder::add_attribute(
		uint32_t location,
		uint32_t binding,
		uint32_t offset,
		AttributeType type
) {
	VkVertexInputAttributeDescription attribute_description = {};
	attribute_description.location = location;
	attribute_description.binding = binding;
	attribute_description.offset = offset;
	attribute_description.format = convert::convert_attribute_format(type);
	attributes.push_back(attribute_description);

	return this;
}

AbstractGraphicShaderBuilder *VulkanGraphicShaderBuilder::add_stage(
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

	if(stage & STAGE_VERTEX)
		stage_bits |= VK_SHADER_STAGE_VERTEX_BIT;

	if(stage & STAGE_FRAGMENT)
		stage_bits |= VK_SHADER_STAGE_FRAGMENT_BIT;

	VkPipelineShaderStageCreateInfo shader_stage_info = {};
	shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_info.module = shader_module;
	shader_stage_info.stage = static_cast<VkShaderStageFlagBits>(stage_bits);
	shader_stage_info.pName = "main";

	shader_stages.push_back(shader_stage_info);

	return this;
}

AbstractGraphicShaderBuilder *VulkanGraphicShaderBuilder::add_layout(LayoutHandle layout_handle) {
	auto layout_instance = layout_manager->get_layout(layout_handle);
	descriptor_layouts.push_back(layout_instance->get_layout());
	return this;
}

AbstractGraphicShaderBuilder *VulkanGraphicShaderBuilder::add_attachment(ImageFormat format) {
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