#include "globals.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h> // haunting cpp file...

#include "tools.h"
#include "pipeline.h"
#include "primitives.h"
#include "vulkan.h"

#include "fs.h"

using namespace render;

VulkanRenderer::VulkanRenderer(GameGlobals *game) {
	this->game = game;
}

static EMesh triangle_mesh;

VulkanRenderer::~VulkanRenderer() {
	vkQueueWaitIdle(present_queue);

	triangle_mesh.destroy(allocator);

	for(auto it = deletion_queue.rbegin(); it != deletion_queue.rend(); it++) {
		(*it)();
	}
	deletion_queue.clear();

}

bool VulkanRenderer::setup() {
	if(!create_vk_instance()) 		return false;
	if(!create_surface()) 			return false;
	if(!create_device()) 			return false;
	if(!create_swapchain()) 		return false;
	if(!create_queues())			return false;
	if(!create_render_pass())		return false;
	if(!create_pipeline_cache())	return false;
	if(!create_descriptor_layout()) return false;
	if(!create_pipelines())			return false;
	if(!create_framebuffers())		return false;
	if(!create_vma_allocator())		return false;
	if(!create_descriptor_pool())	return false;
	if(!create_uniform_buffers())	return false;
	if(!create_descriptor_sets())	return false;
	if(!create_command_pool())		return false;
	if(!create_sync_objects())		return false;

	if(!create_imgui())				return false;

	triangle_mesh.verticies = {
 		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
	};

	triangle_mesh.indicies = {
		 0, 1, 2, 2, 3, 0
	};

	triangle_mesh.allocate(allocator);


	return true;
}

bool VulkanRenderer::draw() {
	ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

	ImGui::ShowDemoWindow();

	ImGuiIO& io = ImGui::GetIO();

	ImGui::GetForegroundDrawList()->AddText(ImVec2(0,0), ImColor(255,255,255), "Epico Engine Text Rendering!!!");
	ImGui::GetForegroundDrawList()->AddText(ImVec2(0,14), ImColor(255,255,255), fmt::format("Rendering at {:.2f}ms ({:.0f} fps)", 1000 / io.Framerate, io.Framerate).c_str());

	ImGui::Render();

	VK_CHECK_BOOL(vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX));

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
	{
		vkCmdSetViewport(command_buffers[image_index], 0, 1, &viewport);
		vkCmdSetScissor(command_buffers[image_index], 0, 1, &scissor);

		vkCmdBeginRenderPass(command_buffers[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
		{
			vkCmdBindPipeline(command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines["vertex"]);

			VkBuffer vertex_buffer[] = { triangle_mesh.vertex_buffer };
			VkDeviceSize offsets[] = { 0 };

			// draw vertex buffer
			vkCmdBindVertexBuffers(command_buffers[image_index], 0, 1, vertex_buffer, offsets);
			vkCmdBindIndexBuffer(command_buffers[image_index], triangle_mesh.index_buffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(command_buffers[image_index], static_cast<uint32_t>(triangle_mesh.indicies.size()), 1, 0, 0, 0);

			// draw triangle
			//vkCmdBindPipeline(command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines["triangle"]);
			//vkCmdDraw(command_buffers[image_index], 3, 1, 0, 0);

			// draw imgui
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffers[image_index]);
		}
		vkCmdEndRenderPass(command_buffers[image_index]);
	}
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

VkShaderModule VulkanRenderer::create_shader(const std::vector<uint32_t> &code) {
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

static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT serverity,
	VkDebugUtilsMessageTypeFlagsEXT message_types,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void *user_data
) {
	UNUSED(message_types);
	UNUSED(user_data);

	switch(serverity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: 		spdlog::info(callback_data->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: 	spdlog::warn(callback_data->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: 	spdlog::error(callback_data->pMessage); break;
	default: break;
	}

	return VK_FALSE;
}

bool VulkanRenderer::create_vk_instance() {
	vkb::InstanceBuilder instance_builder;
	auto instance_builder_ret = instance_builder
									.set_app_name("Epico")
									.set_engine_name("Epico Engine")
									.require_api_version(1,0,0)
									//.request_validation_layers()
									.set_debug_callback(vk_debug_callback)
									.build();

	if(!instance_builder_ret) {
		spdlog::error("Failed to create Vulkan instance {}", instance_builder_ret.error().message());
		return false;
	}

	instance = instance_builder_ret.value();

	deletion_queue.push_back([=]() {
		vkb::destroy_instance(instance);
	});

	return true;
}

bool VulkanRenderer::create_surface() {
	if (!SDL_Vulkan_CreateSurface(game->window, instance.instance, &surface)) {
		spdlog::error("Failed to create SDL Surface {}", SDL_GetError());
		return false;
	}

	deletion_queue.push_back([=]() {
		vkb::destroy_surface(instance, surface);
	});

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

	deletion_queue.push_back([=]() {
		vkb::destroy_device(device);
	});

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

	deletion_queue.push_back([=]() {
		swapchain.destroy_image_views(swapchain_image_views);
		vkb::destroy_swapchain(swapchain);
	});

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

	deletion_queue.push_back([=]() {
		vkDestroyRenderPass(device, render_pass, nullptr);
	});

	return true;
}

bool VulkanRenderer::create_pipeline_cache() {
	VkPipelineCacheCreateInfo pipeline_cache_create_info  = {};
	pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	VK_CHECK_BOOL(vkCreatePipelineCache(device, &pipeline_cache_create_info, nullptr, &pipeline_cache));

	deletion_queue.push_back([=]() {
		vkDestroyPipelineCache(device, pipeline_cache, nullptr);
	});

	return true;
}


bool VulkanRenderer::create_descriptor_layout() {
	VkDescriptorSetLayoutBinding ubo_layout_binding = {};

	ubo_layout_binding.binding = 0;
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo layout_info = {};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = 1;
	layout_info.pBindings = &ubo_layout_binding;

	VK_CHECK_BOOL(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &descriptor_set_layout));

	deletion_queue.push_back([=]() {
		vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
	});

	return true;
}

PipelinePair VulkanRenderer::build_triangle_pipeline() {
	PipelinePair pair = {};
	RenderPipelineConstructor pipeline_constructor(device, render_pass, pipeline_cache);

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

	VkShaderModule triangle_vert = create_shader(fs::read_asset<uint32_t>("shaders/triangle.vert.spv", true));
	VkShaderModule triangle_frag = create_shader(fs::read_asset<uint32_t>("shaders/triangle.frag.spv", true));

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

PipelinePair VulkanRenderer::build_vertex_pipeline() {
	PipelinePair pair = {};
	RenderPipelineConstructor pipeline_constructor(device, render_pass, pipeline_cache);

	//pipeline_constructor.pipeline_layout_info.setLayoutCount = 1;
	//pipeline_constructor.pipeline_layout_info.pSetLayouts = &descriptor_set_layout;

	pipeline_constructor.pipeline_layout_info.setLayoutCount = 0;
	pipeline_constructor.pipeline_layout_info.pushConstantRangeCount = 0;

	auto binding_description = EVertex::get_binding_description();
	auto attribute_descriptions = EVertex::get_attribute_descriptions();

	pipeline_constructor.input_info.vertexBindingDescriptionCount = 1;
	pipeline_constructor.input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());

	pipeline_constructor.input_info.pVertexBindingDescriptions = &binding_description;
	pipeline_constructor.input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

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

	VkShaderModule vertex_vert = create_shader(fs::read_asset<uint32_t>("shaders/vertex.vert.spv", true));
	VkShaderModule vertex_frag = create_shader(fs::read_asset<uint32_t>("shaders/vertex.frag.spv", true));

	pipeline_constructor.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipeline_constructor.input_assembly.primitiveRestartEnable = VK_FALSE;

	pipeline_constructor.add_shader(VK_SHADER_STAGE_VERTEX_BIT, vertex_vert);
	pipeline_constructor.add_shader(VK_SHADER_STAGE_FRAGMENT_BIT, vertex_frag);

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

	vkDestroyShaderModule(device, vertex_vert, nullptr);
	vkDestroyShaderModule(device, vertex_frag, nullptr);

	return pair;
}

PipelinePair VulkanRenderer::build_imgui_pipeline() {
	PipelinePair pair = {};
	RenderPipelineConstructor pipeline_constructor(device, render_pass, pipeline_cache);

	VkShaderModule ui_vert = create_shader(fs::read_asset<uint32_t>("shaders/ui.vert.spv", true));
	VkShaderModule ui_frag = create_shader(fs::read_asset<uint32_t>("shaders/ui.frag.spv", true));

	vkDestroyShaderModule(device, ui_vert, nullptr);
	vkDestroyShaderModule(device, ui_frag, nullptr);

	return pair;
}

bool VulkanRenderer::create_pipelines() {
	pipelines["triangle"] 	= { build_triangle_pipeline() };
	pipelines["vertex"] 	= { build_vertex_pipeline() };
	pipelines["imgui"] 		= { build_imgui_pipeline() };

	{
		size_t size = 0;

		VK_CHECK_BOOL(vkGetPipelineCacheData(device, pipeline_cache, &size, nullptr));

		std::vector<char> pipeline_data(size);
		VK_CHECK_BOOL(vkGetPipelineCacheData(device, pipeline_cache, &size, static_cast<void*>(pipeline_data.data())));

		std::ofstream file("pipeline_cache_data.bin", std::ofstream::out | std::ofstream::binary);

		std::copy(pipeline_data.cbegin(), pipeline_data.cend(), std::ostream_iterator<char>(file));

		file.close();
	}

	deletion_queue.push_back([=] {
		for(auto &map: pipelines) {
			vkDestroyPipeline(device, map.second, nullptr);
			vkDestroyPipelineLayout(device, map.second, nullptr);
		}
	});

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

	deletion_queue.push_back([=]() {
		for(auto framebuffer: framebuffers)
			vkDestroyFramebuffer(device, framebuffer, nullptr);
	});

	return true;
}

bool VulkanRenderer::create_vma_allocator() {
	VmaAllocatorCreateInfo allocator_info = {};
    allocator_info.physicalDevice = device.physical_device;
    allocator_info.device = device;
    allocator_info.instance = instance;

	VK_CHECK_BOOL(vmaCreateAllocator(&allocator_info, &allocator));

	deletion_queue.push_back([=]() {
		vmaDestroyAllocator(allocator);
	});

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

	deletion_queue.push_back([=]() {
		vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
	});

	return true;
}

bool VulkanRenderer::create_uniform_buffers() {
	uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = sizeof(EUniformBufferObject);
        buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		VmaAllocationCreateInfo allocate_info = {};
        allocate_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		allocate_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		allocate_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

		VmaAllocationInfo alloc_info = {};

		vmaCreateBuffer(allocator, &buffer_info, &allocate_info, &uniform_buffers[i].memory.buffer, &uniform_buffers[i].memory.allocation, &uniform_buffers[i].info);
	}

	deletion_queue.push_back([=]() {
		for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			vmaDestroyBuffer(allocator, uniform_buffers[i].memory.buffer, uniform_buffers[i].memory.allocation);
	});

	return true;
}

bool VulkanRenderer::create_descriptor_sets() {

	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptor_set_layout);

	VkDescriptorSetAllocateInfo descriptor_set_info = {};
	descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_info.descriptorPool = descriptor_pool;
	descriptor_set_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	descriptor_set_info.pSetLayouts = layouts.data();

	descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);
	//VK_CHECK_BOOL(vkAllocateDescriptorSets(device, &descriptor_set_info, descriptor_sets.data()));

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo buffer_info = {};
		buffer_info.buffer = uniform_buffers[i].memory.buffer;
		buffer_info.offset = 0;
		buffer_info.range = sizeof(EUniformBufferObject);

		VkWriteDescriptorSet descriptor_write = {};
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = descriptor_sets[i];
		descriptor_write.dstBinding = 0;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_write.descriptorCount = 1;
		descriptor_write.pBufferInfo = &buffer_info;

		//vkUpdateDescriptorSets(device, 1, &descriptor_write, 0, nullptr);
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

	deletion_queue.push_back([=]() {
		vkDestroyCommandPool(device, command_pool, nullptr);
	});

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

	deletion_queue.push_back([=]() {
		for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, finished_semaphores[i], nullptr);
			vkDestroySemaphore(device, available_semaphores[i], nullptr);
			vkDestroyFence(device, in_flight_fences[i], nullptr);
		}
	});

	return true;
}

bool VulkanRenderer::create_imgui() {
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

	deletion_queue.push_back([=]() {
		ImGui_ImplVulkan_Shutdown();
    	ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
	});

	return true;
}

bool VulkanRenderer::rebuild_swapchain() {
	spdlog::debug("Rebuilding swapchain");

	int width = 0, height = 0;
	SDL_GetWindowSize(game->window, &width, &height);


	spdlog::info("{}, {}", width, height);

	vkDeviceWaitIdle(device);

	vkb::destroy_swapchain(swapchain);

	vkDestroyCommandPool(device, command_pool, nullptr);

	for(auto framebuffer: framebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);

	swapchain.destroy_image_views(swapchain_image_views);

	if(!create_swapchain()) 		return false;
	if(!create_framebuffers()) 		return false;
	if(!create_command_pool()) 		return false;

	// HACK: above create commands push a deleter onto the queue, we need to pop them so they don't double free
	for(size_t i = 0; i <= 2; i++)
		deletion_queue.pop_back();

	return true;
}
