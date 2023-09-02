#include "globals.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h> // haunting cpp file...

#include "tools.h"
#include "info.h"
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
	if(!create_vma_allocator())		return false;
	if(!create_depth_image())		return false;
	if(!create_framebuffers())		return false;
	if(!create_descriptor_pool())	return false;
	if(!create_uniform_buffers())	return false;
	if(!create_descriptor_sets())	return false;
	if(!create_command_pool())		return false;
	if(!create_sync_objects())		return false;

	if(!create_imgui())				return false;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "./assets/teapot.obj", nullptr);

	for(const auto& shape: shapes) {
		for (const auto& index : shape.mesh.indices) {
			EVertex vertex = {};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
    			attrib.vertices[3 * index.vertex_index + 1],
    			attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.color = {
				attrib.normals[3 * index.normal_index + 0],
    			attrib.normals[3 * index.normal_index + 1],
    			attrib.normals[3 * index.normal_index + 2]
			};

			triangle_mesh.verticies.push_back(vertex);
			triangle_mesh.indicies.push_back((uint32_t)triangle_mesh.indicies.size());
		}
	}

	triangle_mesh.allocate(allocator);

	submit_command([=](VkCommandBuffer command) {
		triangle_mesh.send_to_gpu(allocator, command);
	});

	triangle_mesh.cleanup_after_send(allocator);

	deletion_queue.push_back([=]() {
		triangle_mesh.destroy(allocator);
	});


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

	static float position_floats[3] = {};
	static float rotation_floats[3] = {};


	ImGui::Begin("Balls");

	ImGui::SliderFloat3("Position", position_floats, -1024, 1024);
	ImGui::SliderFloat3("Rotation", rotation_floats, 0, 360);

	ImGui::End();

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

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VK_CHECK_BOOL(vkBeginCommandBuffer(command_buffers[image_index], &begin_info));
	{
		std::array<VkClearValue, 2> clear_values = {};
		clear_values[0].color = { { 0.2f, 0.2f, 0.2f, 1.0f } };
		clear_values[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = render_pass;
		render_pass_info.framebuffer = framebuffers[image_index];
		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = swapchain.extent;
		render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(command_buffers[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
		{
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

			vkCmdSetViewport(command_buffers[image_index], 0, 1, &viewport);
			vkCmdSetScissor(command_buffers[image_index], 0, 1, &scissor);

			vkCmdBindPipeline(command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines["vertex"]);

			VkBuffer vertex_buffer[] = { triangle_mesh.vertex_buffer };
			VkDeviceSize offsets[] = { 0 };

			UNUSED(vertex_buffer);
			UNUSED(offsets);

			ECameraData ubo = {};

			static auto start_time = std::chrono::high_resolution_clock::now();

        	auto current_time = std::chrono::high_resolution_clock::now();
        	float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

			UNUSED(time);

			// REMINDER: -3.0f is old...

			static glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 0.0f);
			static glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
			static glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
			static glm::vec3 camera_right = glm::normalize(glm::cross(camera_front, camera_up));

			int mx = 0, my = 0;

			SDL_PumpEvents();
			const Uint32 mouse_state =SDL_GetMouseState(&mx, &my);
			const Uint8* key_state = SDL_GetKeyboardState(NULL);

			static float pitch = 0.0f;
			static float yaw = -90.0f;
			const float sensitivity = 0.1f;

			static float last_mx = 400.0f, last_my = 300.0f;

			float offset_mx = (float)mx - last_mx;
			float offset_my = last_my - (float)my;

			last_mx = (float)mx;
			last_my = (float)my;

			if(mouse_state & SDL_BUTTON(3)) {
				offset_mx *= sensitivity;
				offset_my *= sensitivity;

				yaw += offset_mx;
				pitch += offset_my;
			}

			static glm::vec3 camera_direction;

			camera_direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			camera_direction.y = sin(glm::radians(pitch));
			camera_direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

			camera_front = glm::normalize(camera_direction);
			camera_right = glm::normalize(glm::cross(camera_direction, camera_up));

			static const float camera_speed = 0.005f;

			if(key_state[SDL_SCANCODE_W])
				camera_position += camera_front * camera_speed;

			if(key_state[SDL_SCANCODE_S])
				camera_position -= camera_front * camera_speed;

			if(key_state[SDL_SCANCODE_D])
				camera_position += camera_right * camera_speed;

			if(key_state[SDL_SCANCODE_A])
				camera_position -= camera_right * camera_speed;

			ubo.view = glm::lookAt(camera_position, camera_position + camera_front, camera_up);

        	ubo.projection = glm::perspective(glm::radians(90.0f), (float)swapchain.extent.width / (float)swapchain.extent.height, 0.1f, 100.0f);
        	ubo.projection[1][1] *= -1;

			memcpy(camera_data_buffers[current_frame].info.pMappedData, &ubo, sizeof(ECameraData));

			const auto ssbo = static_cast<EObjectData*>(object_data_buffers[current_frame].info.pMappedData);

			ssbo[0].model = calculate_object_matrix(glm::vec3(0, 0, 0), glm::vec3(0, 0.0f, 0), glm::vec3(1, 1, 1));
			ssbo[1].model = calculate_object_matrix(glm::vec3(0, sin(time), 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));

			// draw vertex buffer
			vkCmdBindVertexBuffers(command_buffers[image_index], 0, 1, vertex_buffer, offsets);
			vkCmdBindIndexBuffer(command_buffers[image_index], triangle_mesh.index_buffer, 0, VK_INDEX_TYPE_UINT32);

			// write global, object sets
			vkCmdBindDescriptorSets(command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines["vertex"], 0, 1, &global_descriptor_sets[current_frame], 0, nullptr);
			vkCmdBindDescriptorSets(command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines["vertex"], 1, 1, &object_descriptor_sets[current_frame], 0, nullptr);

			vkCmdDrawIndexed(command_buffers[image_index], static_cast<uint32_t>(triangle_mesh.indicies.size()), 1, 0, 0, 0);
			vkCmdDrawIndexed(command_buffers[image_index], static_cast<uint32_t>(triangle_mesh.indicies.size()), 1, 0, 0, 1);

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
									.require_api_version(1,1,0)
									.request_validation_layers()
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
								.set_minimum_version(1, 1)
								.set_surface(surface)
								.select();

	if(!device_selector_ret) {
		spdlog::error("Failed to create Vulkan Device Selector {}", device_selector_ret.error().message());
		return false;
	}

	VkPhysicalDeviceShaderDrawParametersFeatures draw_features = {};
	draw_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
	draw_features.shaderDrawParameters = VK_TRUE;

	vkb::DeviceBuilder device_builder { device_selector_ret.value() };
	auto device_builder_ret = device_builder.add_pNext(&draw_features).build();

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
	VkSurfaceFormatKHR image_format = {};
	image_format.format = VK_FORMAT_R8G8B8A8_SRGB;
	image_format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

	vkb::SwapchainBuilder swapchain_builder { device };
	auto swapchain_builder_ret = swapchain_builder
									.set_old_swapchain(swapchain)
									.set_desired_format(image_format)
									.build();

	if(!swapchain_builder_ret) {
		spdlog::error("Failed to create Vulkan Swapchain {}", swapchain_builder_ret.error().message());
		return false;
	}

	// destroys the last swapchain during a rebuild
	vkb::destroy_swapchain(swapchain);

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

	VkAttachmentDescription depth_attachment = {};
	depth_attachment.format = find_depth_format();
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::array<VkAttachmentDescription, 2> attachments = {
		color_attachment,
		depth_attachment
	};

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
	render_pass_info.pAttachments = attachments.data();
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
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings = {
			info::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0)
		};

		VkDescriptorSetLayoutCreateInfo layout_info = {};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
		layout_info.pBindings = bindings.data();

		VK_CHECK_BOOL(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &global_descriptor_layout));
	}
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings = {
			info::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0)
		};

		VkDescriptorSetLayoutCreateInfo layout_info = {};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
		layout_info.pBindings = bindings.data();

		VK_CHECK_BOOL(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &object_descriptor_layout));
	}

	deletion_queue.push_back([=]() {
		vkDestroyDescriptorSetLayout(device, object_descriptor_layout, nullptr);
		vkDestroyDescriptorSetLayout(device, global_descriptor_layout, nullptr);
	});

	return true;
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

bool VulkanRenderer::create_depth_image() {
	VkFormat depth_format = find_depth_format();

	VkImageCreateInfo image_create_info = {};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.extent.width = swapchain.extent.width;
	image_create_info.extent.height = swapchain.extent.height;
	image_create_info.extent.depth = 1;
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.format = depth_format;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	auto allocate_info = info::allocation_create_info(VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT);

	VK_CHECK_BOOL(vmaCreateImage(allocator, &image_create_info, &allocate_info, &depth_image.image, &depth_image.allocation, nullptr));

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = depth_image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = depth_format;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	VK_CHECK_BOOL(vkCreateImageView(device, &view_info, nullptr, &depth_image.view));

	deletion_queue.push_back([=]() {
		vkDestroyImageView(device, depth_image.view, nullptr);
		vmaDestroyImage(allocator, depth_image.image, depth_image.allocation);
	});

	return true;
}

bool VulkanRenderer::create_framebuffers() {
	VkFramebufferCreateInfo framebuffer_info = {};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.renderPass = render_pass;
	framebuffer_info.width = swapchain.extent.width;
	framebuffer_info.height = swapchain.extent.height;
	framebuffer_info.layers = 1;

	size_t swapchain_image_count = swapchain_images.size();
	framebuffers.resize(swapchain_images.size());

 	for(size_t i = 0; i < swapchain_image_count; i++) {
		std::array<VkImageView, 2> attachments = {
			swapchain_image_views[i],
			depth_image
		};

		framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebuffer_info.pAttachments = attachments.data();

		VK_CHECK_BOOL(vkCreateFramebuffer(device, &framebuffer_info, nullptr, &framebuffers[i]));

		spdlog::debug("New Framebuffer @ {}", (void*)framebuffers[i]);
	}

	deletion_queue.push_back([=]() {
		for(auto framebuffer: framebuffers)
			vkDestroyFramebuffer(device, framebuffer, nullptr);
	});

	return true;
}

bool VulkanRenderer::create_descriptor_pool() {
	{
		std::vector<VkDescriptorPoolSize> pool_sizes = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 },
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();
		pool_info.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 2;

		VK_CHECK_BOOL(vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool));
	}

	{
		VkDescriptorPoolSize pool_size = {};
		pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pool_size.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = 1;
		pool_info.pPoolSizes = &pool_size;
		pool_info.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VK_CHECK_BOOL(vkCreateDescriptorPool(device, &pool_info, nullptr, &imgui_descriptor_pool));
	}

	deletion_queue.push_back([=]() {
		vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
		vkDestroyDescriptorPool(device, imgui_descriptor_pool, nullptr);
	});

	return true;
}

bool VulkanRenderer::create_uniform_buffers() {
	camera_data_buffers.resize(MAX_FRAMES_IN_FLIGHT);
	object_data_buffers.resize(MAX_FRAMES_IN_FLIGHT);

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		{
			auto buffer_info = info::buffer_create_info(sizeof(ECameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
			auto allocate_info = info::allocation_create_info(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
																VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			VK_CHECK_BOOL(vmaCreateBuffer(allocator, &buffer_info, &allocate_info, &camera_data_buffers[i].memory.buffer, &camera_data_buffers[i].memory.allocation, &camera_data_buffers[i].info));
		}

		{
			auto buffer_info = info::buffer_create_info(sizeof(EObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			auto allocate_info = info::allocation_create_info(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
																VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			VK_CHECK_BOOL(vmaCreateBuffer(allocator, &buffer_info, &allocate_info, &object_data_buffers[i].memory.buffer, &object_data_buffers[i].memory.allocation, &object_data_buffers[i].info));
		}

		deletion_queue.push_back([=]() {
			vmaDestroyBuffer(allocator, object_data_buffers[i].memory.buffer, object_data_buffers[i].memory.allocation);
			vmaDestroyBuffer(allocator, camera_data_buffers[i].memory.buffer, camera_data_buffers[i].memory.allocation);
		});
	}

	return true;
}

bool VulkanRenderer::create_descriptor_sets() {
	global_descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);
	object_descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);

	std::vector<VkDescriptorSetLayout> camera_layouts(MAX_FRAMES_IN_FLIGHT, global_descriptor_layout);
	{
		VkDescriptorSetAllocateInfo camera_descriptor_set_info = {};
		camera_descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		camera_descriptor_set_info.descriptorPool = descriptor_pool;
		camera_descriptor_set_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		camera_descriptor_set_info.pSetLayouts = camera_layouts.data();

		VK_CHECK_BOOL(vkAllocateDescriptorSets(device, &camera_descriptor_set_info, global_descriptor_sets.data()));
	}

	std::vector<VkDescriptorSetLayout> object_layouts(MAX_FRAMES_IN_FLIGHT, object_descriptor_layout);
	{
		VkDescriptorSetAllocateInfo object_descriptor_set_info = {};
		object_descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		object_descriptor_set_info.descriptorPool = descriptor_pool;
		object_descriptor_set_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		object_descriptor_set_info.pSetLayouts = object_layouts.data();

		VK_CHECK_BOOL(vkAllocateDescriptorSets(device, &object_descriptor_set_info, object_descriptor_sets.data()));
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		// write global sets
		VkDescriptorBufferInfo buffer_info = {};
		buffer_info.buffer = camera_data_buffers[i].memory.buffer;
		buffer_info.offset = 0;
		buffer_info.range = sizeof(ECameraData);

		VkWriteDescriptorSet camera_descriptor_write = {};
		camera_descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		camera_descriptor_write.dstSet = global_descriptor_sets[i];
		camera_descriptor_write.dstBinding = 0;
		camera_descriptor_write.dstArrayElement = 0;
		camera_descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		camera_descriptor_write.descriptorCount = 1;
		camera_descriptor_write.pBufferInfo = &buffer_info;


		// write object sets
		VkDescriptorBufferInfo object_buffer_info = {};
		object_buffer_info.buffer = object_data_buffers[i].memory.buffer;
		object_buffer_info.offset = 0;
		object_buffer_info.range = sizeof(EObjectData) * MAX_OBJECTS;

		VkWriteDescriptorSet object_descriptor_write = {};
		object_descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		object_descriptor_write.dstSet = object_descriptor_sets[i];
		object_descriptor_write.dstBinding = 0;
		object_descriptor_write.dstArrayElement = 0;
		object_descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		object_descriptor_write.descriptorCount = 1;
		object_descriptor_write.pBufferInfo = &object_buffer_info;

		VkWriteDescriptorSet descriptor_writes[] = { camera_descriptor_write, object_descriptor_write };

		vkUpdateDescriptorSets(device, 2, descriptor_writes, 0, nullptr);
	}

	return true;
}

bool VulkanRenderer::create_command_pool() {
	command_buffers.resize(framebuffers.size());

	auto command_pool_info = info::command_pool_create_info(graphics_queue_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	VK_CHECK_BOOL(vkCreateCommandPool(device, &command_pool_info, nullptr, &command_pool));

	auto command_allocate_info = info::command_buffer_allocate_info(command_pool, static_cast<uint32_t>(command_buffers.size()));
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
	init_info.DescriptorPool = imgui_descriptor_pool;
	init_info.Subpass = 0;
	init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
	init_info.ImageCount = (uint32_t)framebuffers.size();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr;

	ImGui_ImplVulkan_Init(&init_info, render_pass);

	submit_command([=](VkCommandBuffer command) {
		ImGui_ImplVulkan_CreateFontsTexture(command);
	});

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

	vkDeviceWaitIdle(device);

	vkDestroyCommandPool(device, command_pool, nullptr);

	for(auto framebuffer: framebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);

	vkDestroyImageView(device, depth_image.view, nullptr);
	vmaDestroyImage(allocator, depth_image.image, depth_image.allocation);

	swapchain.destroy_image_views(swapchain_image_views);

	if(!create_swapchain()) 		return false;
	if(!create_depth_image())		return false;
	if(!create_framebuffers()) 		return false;
	if(!create_command_pool()) 		return false;


	// HACK: above create commands push a deleter onto the queue, we need to pop them so they don't double free
	for(size_t i = 0; i <= 3; i++)
		deletion_queue.pop_back();

	return true;
}

VkFormat VulkanRenderer::find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for(VkFormat format: candidates) {
		VkFormatProperties properties = {};
		vkGetPhysicalDeviceFormatProperties(device.physical_device, format, &properties);

		if(tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
			return format;
		}

		if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("Couldn't find a supported format");
}

VkFormat VulkanRenderer::find_depth_format() {
	return find_supported_format({
		VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
	}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

glm::mat4 VulkanRenderer::calculate_object_matrix(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale) {
	glm::quat quaternion(rotation);

	glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 rotation_matrix = glm::toMat4(quaternion);
	glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale);


	return translation_matrix * rotation_matrix * scale_matrix;
}

void VulkanRenderer::submit_command(std::function<void(VkCommandBuffer command)> &&function) {
	VkCommandBuffer command = {};

	auto command_buffer_info = info::command_buffer_allocate_info(command_pool);
	VK_CHECK_VOID(vkAllocateCommandBuffers(device, &command_buffer_info, &command));

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK_VOID(vkBeginCommandBuffer(command, &begin_info));

	function(command);

	VK_CHECK_VOID(vkEndCommandBuffer(command));

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command;

	VK_CHECK_VOID(vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE));
	VK_CHECK_VOID(vkQueueWaitIdle(graphics_queue));

	vkFreeCommandBuffers(device, command_pool, 1, &command);
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
	pipeline_constructor.rasterizer.depthBiasConstantFactor = 0.0f;
	pipeline_constructor.rasterizer.depthBiasSlopeFactor = 0.0f;

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

	std::vector<VkDescriptorSetLayout> descriptor_layouts = {
		global_descriptor_layout,
		object_descriptor_layout
	};

	pipeline_constructor.pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_layouts.size());
	pipeline_constructor.pipeline_layout_info.pSetLayouts = descriptor_layouts.data();

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
	pipeline_constructor.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

	pipeline_constructor.depth_stencil.depthTestEnable = VK_TRUE;
	pipeline_constructor.depth_stencil.depthWriteEnable = VK_TRUE;
	pipeline_constructor.depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
	pipeline_constructor.depth_stencil.depthBoundsTestEnable = VK_FALSE;
	pipeline_constructor.depth_stencil.stencilTestEnable = VK_FALSE;

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