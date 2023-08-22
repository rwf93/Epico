#include "globals.h"

#include "tools.h"
#include "pipeline.h"
#include "vulkan.h"

#include "fs.h"

using namespace render;

vulkanRenderer::vulkanRenderer(gameGlobals *game) {
	this->game = game;
}

vulkanRenderer::~vulkanRenderer() {
	vkQueueWaitIdle(present_queue);

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, finished_semaphores[i], nullptr);
		vkDestroySemaphore(device, available_semaphores[i], nullptr);
		vkDestroyFence(device, in_flight_fences[i], nullptr);
	}

	vkDestroyCommandPool(device, command_pool, nullptr);

	for(auto framebuffer: framebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);

	vkDestroyPipelineCache(device, pipeline_cache, nullptr);

	triangle_pipeline.destroy();

	vkDestroyRenderPass(device, render_pass, nullptr);

	swapchain.destroy_image_views(swapchain_image_views);

	vkb::destroy_swapchain(swapchain);
	vkb::destroy_device(device);
	vkb::destroy_surface(instance, surface);
	vkb::destroy_instance(instance);
}

bool vulkanRenderer::create_vk_instance() {
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

bool vulkanRenderer::create_surface() {
	if (!SDL_Vulkan_CreateSurface(game->window, instance.instance, &surface)) {
		spdlog::error("Failed to create SDL Surface {}", SDL_GetError());
		return false;
	}

	return true;
}

bool vulkanRenderer::create_device() {
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

	return true;
}

bool vulkanRenderer::create_swapchain() {
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

bool vulkanRenderer::create_queues() {
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

bool vulkanRenderer::create_render_pass() {
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

	VK_CHECK_RESULT(vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass));

	return true;
}

bool vulkanRenderer::create_framebuffers() {
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
		VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebuffer_info, nullptr, &framebuffers[i]));

		spdlog::debug("New Framebuffer @ {}", (void*)framebuffers[i]);
	}

	return true;
}

bool vulkanRenderer::create_command_pool() {
	command_buffers.resize(framebuffers.size());

	VkCommandPoolCreateInfo command_pool_info = {};
	command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.pNext = nullptr;
	command_pool_info.queueFamilyIndex = graphics_queue_index;
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VK_CHECK_RESULT(vkCreateCommandPool(device, &command_pool_info, nullptr, &command_pool));

	VkCommandBufferAllocateInfo command_allocate_info = {};
	command_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_allocate_info.commandPool = command_pool;
	command_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_allocate_info.commandBufferCount = (uint32_t)command_buffers.size();

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &command_allocate_info, command_buffers.data()));

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

		VK_CHECK_RESULT(vkBeginCommandBuffer(command_buffers[i], &begin_info));

		vkCmdSetViewport(command_buffers[i], 0, 1, &viewport);
		vkCmdSetScissor(command_buffers[i], 0, 1, &scissor);

		vkCmdBeginRenderPass(command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, triangle_pipeline.pipeline);

		vkCmdDraw(command_buffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(command_buffers[i]);

		VK_CHECK_RESULT(vkEndCommandBuffer(command_buffers[i]));
	}
	*/

	return true;
}
bool vulkanRenderer::create_sync_objects() {
	available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphore_info, nullptr, &available_semaphores[i]));
		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphore_info, nullptr, &finished_semaphores[i]));
		VK_CHECK_RESULT(vkCreateFence(device, &fence_info, nullptr, &in_flight_fences[i]));
	}

	return true;
}

bool vulkanRenderer::create_imgui() {
	return true;
}

bool vulkanRenderer::draw() {
	vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &in_flight_fences[current_frame]);

	uint32_t image_index = 0;
	VkResult aquire_result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

	VK_CHECK_RESULT(vkResetCommandBuffer(command_buffers[image_index], 0));

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

	VK_CHECK_RESULT(vkBeginCommandBuffer(command_buffers[image_index], &begin_info));

	vkCmdSetViewport(command_buffers[image_index], 0, 1, &viewport);
	vkCmdSetScissor(command_buffers[image_index], 0, 1, &scissor);
	vkCmdBeginRenderPass(command_buffers[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, triangle_pipeline.pipeline);
	vkCmdDraw(command_buffers[image_index], 3, 1, 0, 0);
	vkCmdEndRenderPass(command_buffers[image_index]);

	VK_CHECK_RESULT(vkEndCommandBuffer(command_buffers[image_index]));

	if(aquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
		return rebuild_swapchain();
	}

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

	VK_CHECK_RESULT(vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]));

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

bool vulkanRenderer::rebuild_swapchain() {
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

bool vulkanRenderer::create_pipeline_cache() {
	VkPipelineCacheCreateInfo pipeline_cache_create_info  = {};
	pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipeline_cache_create_info, nullptr, &pipeline_cache));

	return true;
}

bool vulkanRenderer::create_pipelines() {
	VkShaderModule vert = create_shader(fs::read_asset<char>("shaders/triangle.vert.spv", true));
	VkShaderModule frag = create_shader(fs::read_asset<char>("shaders/triangle.frag.spv", true));

	triangle_pipeline.pipeline_add_shader(VK_SHADER_STAGE_VERTEX_BIT, vert);
	triangle_pipeline.pipeline_add_shader(VK_SHADER_STAGE_FRAGMENT_BIT, frag);

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

	triangle_pipeline.pipeline_add_viewport(viewport);
	triangle_pipeline.pipeline_add_scissor(scissor);

	triangle_pipeline.pipeline_add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT);
	triangle_pipeline.pipeline_add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR);

	triangle_pipeline.build(device, render_pass, pipeline_cache);

	vkDestroyShaderModule(device, vert, nullptr);
	vkDestroyShaderModule(device, frag, nullptr);

	return true;
}

bool vulkanRenderer::setup() {
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

	return true;
}

VkShaderModule vulkanRenderer::create_shader(FUNC_CREATE_SHADER) {
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