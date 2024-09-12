#include "vkdevice.h"
#include "vkgraphicsprogram.h"
#include "vkresourcemanager.h"
#include "vkgraphicsprogrambuilder.h"

VulkanGraphicsProgramBuilder::VulkanGraphicsProgramBuilder(
	VulkanDevice *vkdevice,
	VulkanResourceManager *vkresourcemanager
)
	: device(vkdevice)
	, resource_manager(vkresourcemanager)
{
	clear();
}

VulkanGraphicsProgramBuilder::~VulkanGraphicsProgramBuilder() {
	clear();
}

void VulkanGraphicsProgramBuilder::clear() {
	clear_bindings();
	clear_attributes();
	clear_stages();
	clear_attachments();

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
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_DEPTH_BIAS
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
								->try_get_resource(current_pipeline_layout)
								.value_or(
									resource_manager->try_get_resource(
										resource_manager->create_layout().build()
									).value()
								)
								->get_pipeline_layout();

	auto handle = resource_manager->graphics_program_pool.acquire(new VulkanGraphicsProgram(
		device
	)).value();
	resource_manager->try_get_resource(handle).value()->init(&pipeline_info);

	return handle;
}

GraphicsProgramBuilder &VulkanGraphicsProgramBuilder::set_primitive(PrimitiveMode type) {
	assembly_info.topology = convert::convert_primitive_type(type);
	assembly_info.primitiveRestartEnable = VK_FALSE;

	return *this;
};

GraphicsProgramBuilder &VulkanGraphicsProgramBuilder::set_polygon_mode(PolygonMode mode) {
	rasterizer_info.polygonMode = convert::convert_polygon_mode(mode);
	return *this;
}

GraphicsProgramBuilder &VulkanGraphicsProgramBuilder::set_depth_format(ImageFormat format) {
	depth_format = convert::convert_image_format(format);
	return *this;
}

GraphicsProgramBuilder &VulkanGraphicsProgramBuilder::set_depth_test(
	bool test_enable,
	bool write_enable,
	CompareOp compare
) {
	stencil_info.depthTestEnable = test_enable;
	stencil_info.depthWriteEnable = write_enable;
	stencil_info.depthCompareOp = convert::convert_compare_op(compare);

	return *this;
}

GraphicsProgramBuilder &VulkanGraphicsProgramBuilder::add_binding(
	uint32_t size,
	BindingRate rate,
	uint32_t binding
) {
	VkVertexInputBindingDescription binding_description = {};
	binding_description.binding = binding;
	binding_description.stride = size;
	binding_description.inputRate = convert::convert_binding_rate(rate);

	bindings.push_back(binding_description);

	return *this;
}

GraphicsProgramBuilder &VulkanGraphicsProgramBuilder::set_cull_face(CullFace face) {
	rasterizer_info.cullMode = convert::convert_cull_type(face);
	return *this;
}

GraphicsProgramBuilder &VulkanGraphicsProgramBuilder::set_front_face(FrontFace face) {
	rasterizer_info.frontFace = convert::convert_face_type(face);
	return *this;
}

GraphicsProgramBuilder &VulkanGraphicsProgramBuilder::add_attribute(
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

	return *this;
}

GraphicsProgramBuilder &VulkanGraphicsProgramBuilder::add_stage(
	ShaderHandle handle
) {
	auto resource = resource_manager->try_get_resource(handle).value();
	shader_stages.push_back(resource->stage_info);
	return *this;
}

GraphicsProgramBuilder &VulkanGraphicsProgramBuilder::clear_stages() {
	shader_stages.clear();
	return *this;
}

GraphicsProgramBuilder &VulkanGraphicsProgramBuilder::clear_bindings() {
	bindings.clear();
	return *this;
}

GraphicsProgramBuilder &VulkanGraphicsProgramBuilder::clear_attributes() {
	attributes.clear();
	return *this;
}

GraphicsProgramBuilder &VulkanGraphicsProgramBuilder::clear_attachments() {
	color_states.clear();
	attachment_formats.clear();
	return *this;
}

GraphicsProgramBuilder &VulkanGraphicsProgramBuilder::set_layout(LayoutHandle layout_handle) {
	current_pipeline_layout = layout_handle;
	return *this;
}

GraphicsProgramBuilder &VulkanGraphicsProgramBuilder::add_attachment(ImageFormat format) {
	VkPipelineColorBlendAttachmentState color_blend_state = {};
	color_blend_state.colorWriteMask =
								VK_COLOR_COMPONENT_R_BIT |
								VK_COLOR_COMPONENT_G_BIT |
								VK_COLOR_COMPONENT_B_BIT |
								VK_COLOR_COMPONENT_A_BIT;
	color_states.push_back(color_blend_state);

	attachment_formats.push_back(convert::convert_image_format(format));

	// Update color_info
	color_info.attachmentCount = static_cast<uint32_t>(color_states.size());
	color_info.pAttachments = color_states.data();

	return *this;
}
