#include "vulkan.h"

bool create_vk_instance(FUNC_CREATE_VK_INSTANCE) {
	vkb::InstanceBuilder instance_builder;
	auto instance_builder_ret = instance_builder
									.set_app_name("Epico")
									.set_engine_name("Epico Engine")
									.require_api_version(1,0,0)
									.build();

	if(!instance_builder_ret) {
		fmt::println("Failed to create Vulkan instance {}", instance_builder_ret.error().message());
		return false;
	}

	game->instance = instance_builder_ret.value();

	return true;
}

bool create_surface(FUNC_CREATE_SURFACE) {
	if (!SDL_Vulkan_CreateSurface(game->window, game->instance.instance, &game->surface)) {
		fmt::println("Failed to create SDL Surface {}", SDL_GetError());
		return false;
	}

	return true;
}

bool create_device(FUNC_CREATE_DEVICE) {
	vkb::PhysicalDeviceSelector device_selector(game->instance);
	auto device_selector_ret = device_selector
								.set_surface(game->surface)
								.select();

	if(!device_selector_ret) {
		fmt::println("Failed to create Vulkan Device Selector {}", device_selector_ret.error().message());
		return false;
	}

	vkb::DeviceBuilder device_builder{ device_selector_ret.value() };
	auto device_builder_ret = device_builder.build();

	if(!device_builder_ret) {
		fmt::println("Failed to create Vulkan Device {}", device_builder_ret.error().message());
		return false;
	}

	game->device = device_builder_ret.value(); // ze bluechowth dewice has connectedey suchessfulley

	return true;
}

bool create_swapchain(FUNC_CREATE_SWAPCHAIN) {
	vkb::SwapchainBuilder swapchain_builder{ game->device };
	auto swapchain_builder_ret = swapchain_builder.build();

	if(!swapchain_builder_ret) {
		fmt::println("Failed to create Vulkan Swapchain {}", swapchain_builder_ret.error().message());
		return false;
	}

	game->swapchain = swapchain_builder_ret.value();

	return true;
}

bool get_queues(FUNC_GET_QUEUES) {
	auto gq = game->device.get_queue(vkb::QueueType::graphics);
	auto pq = game->device.get_queue(vkb::QueueType::present);

	if (!gq.has_value()) {
		fmt::println("Failed to create Get Vulkan Graphics Queue");
		return false;
	}

	if (!pq.has_value()) {
		fmt::println("Failed to create Get Vulkan Present Queue");
		return false;
	}

	game->graphics_queue = gq.value();
	game->present_queue = pq.value();

	return true;
}

bool create_render_pass(FUNC_CREATE_RENDER_PASS) {
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = game->swapchain.image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	if(vkCreateRenderPass(game->device, &render_pass_info, nullptr, &game->render_pass) != VK_SUCCESS) {
		fmt::println("Failed to create render pass");
		return false;
	}

	return true;
}

VkShaderModule create_shader(FUNC_CREATE_SHADER) {
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = code.size();
	create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shader_module;
	if(vkCreateShaderModule(game->device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
		fmt::println("Warning: couldn't create shader module");
		return VK_NULL_HANDLE;
	}

	return shader_module;
}

bool create_graphics_pipeline(FUNC_CREATE_GRAPHICS_PIPELINE) {
	VkShaderModule frag = create_shader(game, read_file("assets/shaders/frag.spv", false));
	VkShaderModule vert = create_shader(game, read_file("assets/shaders/vert.spv", false));

	VkPipelineShaderStageCreateInfo vert_stage_info = {};
	vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_stage_info.module = vert;
	vert_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo frag_stage_info = {};
	frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_stage_info.module = frag;
	frag_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages[] = { vert_stage_info, frag_stage_info };

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.vertexAttributeDescriptionCount = 0;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)game->swapchain.extent.width;
	viewport.height = (float)game->swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = game->swapchain.extent;

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &colorBlendAttachment;
	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pushConstantRangeCount = 0;

	if(vkCreatePipelineLayout(game->device, &pipeline_layout_info, nullptr, &game->graphics_pipeline_layout) != VK_SUCCESS) {
		fmt::println("Failed to create graphics pipeline layout");
		return false;
	}

	vkDestroyShaderModule(game->device, frag, nullptr);
	vkDestroyShaderModule(game->device, vert, nullptr);

	return true;
}
