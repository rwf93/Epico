#include "globals.h"

#include "tools.h"
#include "pipeline.h"
#include "vulkan.h"

#include "fs.h"

using namespace render;

VulkanRenderer::VulkanRenderer(GameGlobals *game) {
	this->game = game;
}

VulkanRenderer::~VulkanRenderer() {
	vkQueueWaitIdle(present_queue);

	ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	vkDestroyDescriptorPool(device, descriptor_pool, nullptr);

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, finished_semaphores[i], nullptr);
		vkDestroySemaphore(device, available_semaphores[i], nullptr);
		vkDestroyFence(device, in_flight_fences[i], nullptr);
	}

	vkDestroyCommandPool(device, command_pool, nullptr);

	for(auto framebuffer: framebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);

	//vkDestroyPipeline(device, triangle_pipeline, nullptr);
	//vkDestroyPipelineLayout(device, pipeline_layout, nullptr);

	for(auto &map: pipelines) {
		vkDestroyPipeline(device, map.second, nullptr);
		vkDestroyPipelineLayout(device, map.second, nullptr);
	}

	vkDestroyPipelineCache(device, pipeline_cache, nullptr);

	vkDestroyRenderPass(device, render_pass, nullptr);

	swapchain.destroy_image_views(swapchain_image_views);

	vkb::destroy_swapchain(swapchain);
	vkb::destroy_device(device);
	vkb::destroy_surface(instance, surface);
	vkb::destroy_instance(instance);
}

bool VulkanRenderer::setup() {
	if(!create_vk_instance()) 		return false;
	if(!create_surface()) 			return false;
	if(!create_device()) 			return false;
	if(!create_swapchain()) 		return false;
	if(!create_queues())			return false;
	if(!create_render_pass())		return false;
	if(!create_pipeline_cache())	return false;
	if(!create_pipelines())			return false; // remove l8r
	if(!create_framebuffers())		return false;
	if(!create_command_pool())		return false;
	if(!create_sync_objects())		return false;
	if(!create_descriptor_pool())	return false;

	if(!setup_imgui())				return false;

	return true;
}

bool VulkanRenderer::draw() {
	ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

	ImGui::GetForegroundDrawList()->AddText(ImVec2(0,0), ImColor(255,255,255), "Epico Engine Text Rendering!!!\n");

	ImGui::Render();

	VK_CHECK_BOOL(vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX));
	VK_CHECK_BOOL(vkResetFences(device, 1, &in_flight_fences[current_frame]));

	uint32_t image_index = 0;
	VkResult aquire_result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

	if(aquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
		return rebuild_swapchain();
	}

	VK_CHECK_BOOL(vkResetFences(device, 1, &in_flight_fences[current_frame]));

	VkSemaphore wait_semaphores[] = { available_semaphores[current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSemaphore signal_semaphores[] = { finished_semaphores[current_frame] };

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffers[image_index];
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	/* Begin recording our command buffer, intended on sending it to the GPU */
	VK_CHECK_BOOL(vkResetCommandBuffer(command_buffers[image_index], 0));

	VkClearValue clearColor{ { { 0.0f, 0.0f, 0.0f, 1.0f } } };
	VkRenderPassBeginInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = render_pass;
	render_pass_info.framebuffer = framebuffers[image_index];
	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = swapchain.extent;
	render_pass_info.clearValueCount = 1;
	render_pass_info.pClearValues = &clearColor;
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapchain.extent.width;
	viewport.height = (float)swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchain.extent;
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VK_CHECK_BOOL(vkBeginCommandBuffer(command_buffers[image_index], &begin_info));

	vkCmdSetViewport(command_buffers[image_index], 0, 1, &viewport);
	vkCmdSetScissor(command_buffers[image_index], 0, 1, &scissor);

	vkCmdBeginRenderPass(command_buffers[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffers[image_index]);

	// draw triangle
	vkCmdBindPipeline(command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines["triangle"]);
	vkCmdDraw(command_buffers[image_index], 3, 1, 0, 0);

	vkCmdEndRenderPass(command_buffers[image_index]);

	VK_CHECK_BOOL(vkEndCommandBuffer(command_buffers[image_index]));
	/* End recording our command buffer, intended on sending it to the GPU */

	VK_CHECK_BOOL(vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]));

	VkSwapchainKHR swap_chains[] = { swapchain };

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;
	present_info.pImageIndices = &image_index;

	VkResult present_result = vkQueuePresentKHR(present_queue, &present_info);

	if(present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR) {
		return rebuild_swapchain();
	}

	current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
	return true;
}

VkShaderModule VulkanRenderer::create_shader(FUNC_CREATE_SHADER) {
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = code.size();
	create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shader_module;
	if(vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
		spdlog::error("Warning: couldn't create shader module");
		return VK_NULL_HANDLE;
	}

	return shader_module;
}

bool VulkanRenderer::create_vk_instance() {
	vkb::InstanceBuilder instance_builder;
	auto instance_builder_ret = instance_builder
									.set_app_name("Epico")
									.set_engine_name("Epico Engine")
									.require_api_version(1,0,0)
									.request_validation_layers()
									.use_default_debug_messenger()
									.build();

	if(!instance_builder_ret) {
		spdlog::error("Failed to create Vulkan instance {}", instance_builder_ret.error().message());
		return false;
	}

	instance = instance_builder_ret.value();

	return true;
}

bool VulkanRenderer::create_surface() {
	if (!SDL_Vulkan_CreateSurface(game->window, instance.instance, &surface)) {
		spdlog::error("Failed to create SDL Surface {}", SDL_GetError());
		return false;
	}

	return true;
}

bool VulkanRenderer::create_device() {
	vkb::PhysicalDeviceSelector device_selector(instance);
	auto device_selector_ret = device_selector
								.set_surface(surface)
								.select();

	if(!device_selector_ret) {
		spdlog::error("Failed to create Vulkan Device Selector {}", device_selector_ret.error().message());
		return false;
	}

	vkb::DeviceBuilder device_builder { device_selector_ret.value() };
	auto device_builder_ret = device_builder.build();

	if(!device_builder_ret) {
		spdlog::error("Failed to create Vulkan Device {}", device_builder_ret.error().message());
		return false;
	}

	device = device_builder_ret.value(); // ze bluechowth dewice has connectedey suchessfulley

	spdlog::info("Found capable render device: {}", device.physical_device.name);

	return true;
}

bool VulkanRenderer::create_swapchain() {
	vkb::SwapchainBuilder swapchain_builder { device };
	auto swapchain_builder_ret = swapchain_builder.build();

	if(!swapchain_builder_ret) {
		spdlog::error("Failed to create Vulkan Swapchain {}", swapchain_builder_ret.error().message());
		return false;
	}

	swapchain = swapchain_builder_ret.value();
	swapchain_image_views = swapchain.get_image_views().value();
	swapchain_images = swapchain.get_images().value();

	return true;
}

bool VulkanRenderer::create_queues() {
	auto gq = device.get_queue(vkb::QueueType::graphics);
	auto pq = device.get_queue(vkb::QueueType::present);
	auto gqi = device.get_queue_index(vkb::QueueType::graphics);

	if (!gq.has_value()) {
		spdlog::error("Failed to get Vulkan Graphics Queue");
		return false;
	}

	if(!pq.has_value()) {
		spdlog::error("Failed to get Vulkan Present Queue");
		return false;
	}

	if (!gqi.has_value()) {
		spdlog::error("Failed to get Vulkan Graphics Queue Index");
		return false;
	}

	graphics_queue = gq.value();
	present_queue = pq.value();
	graphics_queue_index = gqi.value();

	return true;
}

bool VulkanRenderer::create_render_pass() {
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = swapchain.image_format;
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

	VK_CHECK_BOOL(vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass));

	return true;
}

bool VulkanRenderer::create_pipeline_cache() {
	VkPipelineCacheCreateInfo pipeline_cache_create_info  = {};
	pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	VK_CHECK_BOOL(vkCreatePipelineCache(device, &pipeline_cache_create_info, nullptr, &pipeline_cache));

	return true;
}

PipelinePair VulkanRenderer::build_triangle_pipeline() {
	PipelinePair pair = {};

	RenderPipelineConstructor pipeline_constructor(device, render_pass, pipeline_cache);

	pipeline_constructor.pipeline_layout_info.setLayoutCount = 0;
	pipeline_constructor.pipeline_layout_info.pushConstantRangeCount = 0;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapchain.extent.width;
	viewport.height = (float)swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchain.extent;

	/* begin simple triangle */

	VkShaderModule triangle_vert = create_shader(fs::read_asset<char>("shaders/triangle.vert.spv", true));
	VkShaderModule triangle_frag = create_shader(fs::read_asset<char>("shaders/triangle.frag.spv", true));

	pipeline_constructor.input_info.vertexBindingDescriptionCount = 0;
	pipeline_constructor.input_info.vertexAttributeDescriptionCount = 0;

	pipeline_constructor.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipeline_constructor.input_assembly.primitiveRestartEnable = VK_FALSE;

	pipeline_constructor.add_shader(VK_SHADER_STAGE_VERTEX_BIT, triangle_vert);
	pipeline_constructor.add_shader(VK_SHADER_STAGE_FRAGMENT_BIT, triangle_frag);

	pipeline_constructor.viewport_state.viewportCount = 1;
	pipeline_constructor.viewport_state.pViewports = &viewport;
	pipeline_constructor.viewport_state.scissorCount = 1;
	pipeline_constructor.viewport_state.pScissors = &scissor;

	pipeline_constructor.rasterizer.depthClampEnable = VK_FALSE;
	pipeline_constructor.rasterizer.rasterizerDiscardEnable = VK_FALSE;
	pipeline_constructor.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	pipeline_constructor.rasterizer.lineWidth = 1.0f;
	pipeline_constructor.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	pipeline_constructor.rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	pipeline_constructor.rasterizer.depthBiasEnable = VK_FALSE;

	pipeline_constructor.multisampling.sampleShadingEnable = VK_FALSE;
	pipeline_constructor.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;

	pipeline_constructor.color_attachments.push_back(color_blend_attachment);

	pipeline_constructor.color_blending.logicOpEnable = VK_FALSE;
	pipeline_constructor.color_blending.logicOp = VK_LOGIC_OP_COPY;

	pipeline_constructor.color_blending.blendConstants[0] = 0.0f;
	pipeline_constructor.color_blending.blendConstants[1] = 0.0f;
	pipeline_constructor.color_blending.blendConstants[2] = 0.0f;
	pipeline_constructor.color_blending.blendConstants[3] = 0.0f;

	pipeline_constructor.dynamic_states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	pipeline_constructor.dynamic_states.push_back(VK_DYNAMIC_STATE_SCISSOR);

	pipeline_constructor.build(&pair.pipeline, &pair.layout);

	vkDestroyShaderModule(device, triangle_vert, nullptr);
	vkDestroyShaderModule(device, triangle_frag, nullptr);

	return pair;
}

bool VulkanRenderer::create_pipelines() {

	pipelines["triangle"] = { build_triangle_pipeline() };

	return true;
}

bool VulkanRenderer::create_framebuffers() {
	VkFramebufferCreateInfo framebuffer_info = {};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.renderPass = render_pass;
	framebuffer_info.attachmentCount = 1;
	framebuffer_info.width = swapchain.extent.width;
	framebuffer_info.height = swapchain.extent.height;
	framebuffer_info.layers = 1;

	size_t swapchain_image_count = swapchain_images.size();
	framebuffers.resize(swapchain_images.size());

 	for(size_t i = 0; i < swapchain_image_count; i++) {
		framebuffer_info.pAttachments = &swapchain_image_views[i];
		VK_CHECK_BOOL(vkCreateFramebuffer(device, &framebuffer_info, nullptr, &framebuffers[i]));

		spdlog::debug("New Framebuffer @ {}", (void*)framebuffers[i]);
	}

	return true;
}

bool VulkanRenderer::create_command_pool() {
	command_buffers.resize(framebuffers.size());

	VkCommandPoolCreateInfo command_pool_info = {};
	command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.pNext = nullptr;
	command_pool_info.queueFamilyIndex = graphics_queue_index;
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VK_CHECK_BOOL(vkCreateCommandPool(device, &command_pool_info, nullptr, &command_pool));

	VkCommandBufferAllocateInfo command_allocate_info = {};
	command_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_allocate_info.commandPool = command_pool;
	command_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_allocate_info.commandBufferCount = (uint32_t)command_buffers.size();

	VK_CHECK_BOOL(vkAllocateCommandBuffers(device, &command_allocate_info, command_buffers.data()));

	/*
	// begin recording the command buffer
	for(size_t i = 0; i < command_buffers.size(); i++) {
		spdlog::debug("Recording to command buffer @ {}", i);

		VkClearValue clearColor{ { { 0.0f, 0.0f, 0.0f, 1.0f } } };
		VkRenderPassBeginInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = render_pass;
		render_pass_info.framebuffer = framebuffers[i];
		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = swapchain.extent;
		render_pass_info.clearValueCount = 1;
		render_pass_info.pClearValues = &clearColor;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchain.extent.width;
		viewport.height = (float)swapchain.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = swapchain.extent;

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VK_CHECK_BOOL(vkBeginCommandBuffer(command_buffers[i], &begin_info));

		vkCmdSetViewport(command_buffers[i], 0, 1, &viewport);
		vkCmdSetScissor(command_buffers[i], 0, 1, &scissor);

		vkCmdBeginRenderPass(command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, triangle_pipeline.pipeline);

		vkCmdDraw(command_buffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(command_buffers[i]);

		VK_CHECK_BOOL(vkEndCommandBuffer(command_buffers[i]));
	}
	*/

	return true;
}
bool VulkanRenderer::create_sync_objects() {
	available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VK_CHECK_BOOL(vkCreateSemaphore(device, &semaphore_info, nullptr, &available_semaphores[i]));
		VK_CHECK_BOOL(vkCreateSemaphore(device, &semaphore_info, nullptr, &finished_semaphores[i]));
		VK_CHECK_BOOL(vkCreateFence(device, &fence_info, nullptr, &in_flight_fences[i]));
	}

	return true;
}

bool VulkanRenderer::create_descriptor_pool() {
	VkDescriptorPoolSize pool_size = {};
	pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_size.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = 1;
	pool_info.pPoolSizes = &pool_size;
	pool_info.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VK_CHECK_BOOL(vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool));

	return true;
}

bool VulkanRenderer::setup_imgui() {

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForVulkan(game->window);

	ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = instance;
	init_info.PhysicalDevice = device.physical_device;
	init_info.Device = device;
	init_info.QueueFamily = graphics_queue_index;
	init_info.Queue = graphics_queue;
	init_info.PipelineCache = pipeline_cache;
	init_info.DescriptorPool = descriptor_pool;
	init_info.Subpass = 0;
	init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
	init_info.ImageCount = (uint32_t)framebuffers.size();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr;

	ImGui_ImplVulkan_Init(&init_info, render_pass);

	/* Upload IMGUI fonts and textures to the GPU */
	VK_CHECK_BOOL(vkResetCommandPool(device, command_pool, 0));

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VK_CHECK_BOOL(vkBeginCommandBuffer(command_buffers[current_frame], &begin_info));

	ImGui_ImplVulkan_CreateFontsTexture(command_buffers[current_frame]);

	VK_CHECK_BOOL(vkEndCommandBuffer(command_buffers[current_frame]));

	VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers[current_frame];

	VK_CHECK_BOOL(vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE));

	vkDeviceWaitIdle(device);

	ImGui_ImplVulkan_DestroyFontUploadObjects();

	return true;
}

bool VulkanRenderer::rebuild_swapchain() {
	spdlog::debug("Rebuilding swapchain");

	vkDeviceWaitIdle(device);

	vkDestroySwapchainKHR(device, swapchain, nullptr);

	vkDestroyCommandPool(device, command_pool, nullptr);

	for(auto framebuffer: framebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);

	swapchain.destroy_image_views(swapchain_image_views);

	if(!create_swapchain()) 		return false;
	if(!create_framebuffers()) 		return false;
	if(!create_command_pool()) 		return false;

	return true;
}
