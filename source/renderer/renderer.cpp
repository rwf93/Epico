#define VMA_IMPLEMENTATION

#include "fs.h"
#include "mathlib.h"

#include "tools.h"
#include "info.h"
#include "pipeline.h"

#include "renderer.h"

using namespace render;

Renderer::Renderer(GameGlobals *game) {
	this->game = game;
}

Renderer::~Renderer() {
	vkQueueWaitIdle(present_queue);

	// evil abuse of functional programming
	std::for_each(deletion_queue.rbegin(), deletion_queue.rend(), [=, this](std::function<void()> &func) { func(); });
	deletion_queue.clear();
}

bool Renderer::setup() {
	if(!create_instance()) 		return false;
	if(!create_surface()) 			return false;
	if(!create_device()) 			return false;
	if(!create_swapchain()) 		return false;
	if(!create_queues())			return false;
	if(!create_pipeline_cache())	return false;
	if(!create_descriptor_layout()) return false;
	if(!create_pipelines())			return false;
	if(!create_vma_allocator())		return false;
	if(!create_depth_image())		return false;
	if(!create_texture_sampler())	return false;
	if(!create_texture_array())		return false;
	if(!create_descriptor_pool())	return false;
	if(!create_uniform_buffers())	return false;
	if(!create_descriptor_sets())	return false;
	if(!create_command_pool())		return false;
	if(!create_sync_objects())		return false;

	if(!create_imgui())				return false;

	meshes["monkey"].load_mesh(allocator, "./assets/models/monkey.glb", this);

	deletion_queue.push_back([&]() {
		for(auto &map: meshes)
			map.second.destroy();
	});

	return true;
}

bool Renderer::begin() {
	VK_CHECK_BOOL(vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX));

	VkResult aquire_result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);
	if(aquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
		return rebuild_swapchain();
	}

	VK_CHECK_BOOL(vkResetFences(device, 1, &in_flight_fences[current_frame]));

	VK_CHECK_BOOL(vkResetCommandBuffer(command_buffers[current_frame], 0));

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VK_CHECK_BOOL(vkBeginCommandBuffer(command_buffers[current_frame], &begin_info));

	{
		image_barrier(
						command_buffers[current_frame], swapchain_images[image_index],
						0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
						VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
						VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
		);

		image_barrier(
						command_buffers[current_frame], depth_image.image,
						0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
						VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
						VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
						VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
						VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 }
		);

		VkRenderingAttachmentInfoKHR color_attachment = {};
		color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		color_attachment.imageView = swapchain_image_views[image_index];
		color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

		VkRenderingAttachmentInfoKHR depth_attachment = {};
		depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		depth_attachment.imageView = depth_image.view;
		depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment.clearValue.color = { 1.0f, 0 };

		VkRenderingInfoKHR rendering_info = {};
		rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		rendering_info.renderArea = { 0, 0, swapchain.extent.width, swapchain.extent.height };
		rendering_info.layerCount = 1;
		rendering_info.colorAttachmentCount = 1;
		rendering_info.pColorAttachments = &color_attachment;
		rendering_info.pDepthAttachment = &depth_attachment;

		vkCmdBeginRenderingKHR(command_buffers[current_frame], &rendering_info);
		{
			VkViewport viewport = info::viewport(static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height));
			VkRect2D scissor = info::rect2d({ 0, 0 }, swapchain.extent);

			vkCmdSetViewport(command_buffers[current_frame], 0, 1, &viewport);
			vkCmdSetScissor(command_buffers[current_frame], 0, 1, &scissor);

			EGlobalData ubo = {};

			static glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 0.0f);
			static glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
			static glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
			static glm::vec3 camera_right = glm::normalize(glm::cross(camera_front, camera_up));

			int mx = 0, my = 0;

			SDL_PumpEvents();
			const Uint32 mouse_state = SDL_GetMouseState(&mx, &my);
			const Uint8* key_state = SDL_GetKeyboardState(NULL);

			static float pitch = 0.0f;
			static float yaw = -90.0f;
			const float sensitivity = 0.1f;

			static float last_mx = 400.0f, last_my = 300.0f;

			float offset_mx = (float)mx - last_mx;
			float offset_my = last_my - (float)my;

			last_mx = static_cast<float>(mx);
			last_my = static_cast<float>(my);

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

			float camera_speed = 0.3f;

			if(key_state[SDL_SCANCODE_LSHIFT])
				camera_speed *= 2.0f;

			if(key_state[SDL_SCANCODE_W])
				camera_position += camera_front * (game->time_delta * camera_speed);

			if(key_state[SDL_SCANCODE_S])
				camera_position -= camera_front * (game->time_delta * camera_speed);

			if(key_state[SDL_SCANCODE_D])
				camera_position += camera_right * (game->time_delta * camera_speed);

			if(key_state[SDL_SCANCODE_A])
				camera_position -= camera_right * (game->time_delta * camera_speed);

			ubo.view = glm::lookAt(camera_position, camera_position + camera_front, camera_up);

			ubo.projection = glm::perspective(glm::radians(90.0f), static_cast<float>(swapchain.extent.width) / static_cast<float>(swapchain.extent.height), 0.01f, 100.0f);
			ubo.projection[1][1] *= -1;

			memcpy(camera_data_buffers[current_frame].info.pMappedData, &ubo, sizeof(EGlobalData));
			const auto ssbo = static_cast<EObjectData*>(object_data_buffers[current_frame].info.pMappedData);

			vkCmdBindDescriptorSets(command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layouts["vertex"], 0, 1, &global_descriptor_sets[current_frame], 0, nullptr);
			vkCmdBindDescriptorSets(command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layouts["vertex"], 1, 1, &object_descriptor_sets[current_frame], 0, nullptr);

			static int render_mode_index = 0;
			static std::vector<const char*> render_modes = {
				"Normal",
				"Wireframe",
				"Points",
				"Phong",
				"Toon"
			};

			switch(render_mode_index) {
				case 0: vkCmdBindPipeline(command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines["vertex"]); break;
				case 1: vkCmdBindPipeline(command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines["vertex_wireframe"]); break;
				case 2: vkCmdBindPipeline(command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines["vertex_points"]); break;
				case 3: vkCmdBindPipeline(command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines["vertex_phong"]); break;
				case 4: vkCmdBindPipeline(command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines["vertex_toon"]); break;
			}

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();

			ImGuiIO &io = ImGui::GetIO();

			ImGui::Begin("Scene Settings");
			{
				if(ImGui::BeginCombo("Render Modes", render_modes[render_mode_index])) {
					for(int n = 0; n < static_cast<int>(render_modes.size()); n++) {
						const bool is_selected = (render_mode_index == n);
						if(ImGui::Selectable(render_modes[n], is_selected))
							render_mode_index = n;

						if(is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			ImGui::End();

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
			{
				ImGui::SetNextWindowPos(ImVec2(1.5f, 1.5f));
				ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoSavedSettings);
				{
					ImGui::Text("Statistics");
					ImGui::Separator();
					ImGui::Text("Frames Per Second: %.1f (%.3fms/frame)", io.Framerate, 1000.0f / io.Framerate);
					ImGui::Text("Surface Size: %ix%i", swapchain.extent.width, swapchain.extent.height);
				}
				ImGui::End();
			}
			ImGui::PopStyleVar();

			ssbo[0].model = mathlib::calculate_model_matrix(glm::vec3(0, 0, -2), glm::vec3(game->time), glm::vec3(0.2, 0.2, 0.2));

			VkDeviceSize offset[] = { 0 };
			vkCmdBindVertexBuffers(command_buffers[current_frame], 0, 1, &meshes["monkey"].vertex_buffer.buffer, offset);
			vkCmdBindIndexBuffer(command_buffers[current_frame], meshes["monkey"].index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(command_buffers[current_frame], meshes["monkey"].index_count, 1, 0, 0, 0);

			ImGui::Render();

			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffers[current_frame]);
		}
		vkCmdEndRenderingKHR(command_buffers[current_frame]);

		image_barrier(
						command_buffers[current_frame], swapchain_images[image_index],
						VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0,
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
						VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
						VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
		);
	}


	return true;
}

bool Renderer::end() {
	VkSemaphore wait_semaphores[] = { available_semaphores[current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSemaphore signal_semaphores[] = { finished_semaphores[current_frame] };

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffers[current_frame];
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	VK_CHECK_BOOL(vkEndCommandBuffer(command_buffers[current_frame]));

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

VkShaderModule Renderer::create_shader(const std::vector<uint32_t> &code) {
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

void Renderer::image_barrier(
						VkCommandBuffer command, VkImage image,
						VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
						VkImageLayout old_image_layout, VkImageLayout new_image_layout,
						VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
						VkImageSubresourceRange resource_range
) {
	VkImageMemoryBarrier image_barrier = {};
	image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	image_barrier.dstQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
	image_barrier.srcAccessMask = src_access_mask;
	image_barrier.dstAccessMask = dst_access_mask;
	image_barrier.oldLayout = old_image_layout;
	image_barrier.newLayout = new_image_layout;
	image_barrier.image = image;
	image_barrier.subresourceRange = resource_range;

	vkCmdPipelineBarrier(command, src_stage_mask, dst_stage_mask, 0, 0, nullptr, 0, nullptr, 1, &image_barrier);
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

bool Renderer::create_instance() {
	vkb::InstanceBuilder instance_builder;
	auto instance_builder_ret = instance_builder
									.set_app_name("Epico")
									.set_engine_name("Epico Engine")
									.require_api_version(1,1,0)
									.request_validation_layers()
									.set_debug_callback(vk_debug_callback)
									.enable_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)
									.build();

	if(!instance_builder_ret) {
		spdlog::error("Failed to create Vulkan instance {}", instance_builder_ret.error().message());
		return false;
	}

	instance = instance_builder_ret.value();

	deletion_queue.push_back([=, this]() {
		vkb::destroy_instance(instance);
	});

	return true;
}

bool Renderer::create_surface() {
	if (!SDL_Vulkan_CreateSurface(game->window, instance.instance, &surface)) {
		spdlog::error("Failed to create SDL Surface {}", SDL_GetError());
		return false;
	}

	deletion_queue.push_back([=, this]() {
		vkb::destroy_surface(instance, surface);
	});

	return true;
}

bool Renderer::create_device() {
	VkPhysicalDeviceFeatures device_features = {};
	device_features.fillModeNonSolid = true;

	std::vector<const char*> extensions = {
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
		VK_KHR_MAINTENANCE2_EXTENSION_NAME,
		VK_KHR_MULTIVIEW_EXTENSION_NAME,
		VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
		VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME
	};

	vkb::PhysicalDeviceSelector device_selector(instance);
	auto device_selector_ret = device_selector
								.set_required_features((const VkPhysicalDeviceFeatures)device_features)
								.add_required_extensions(extensions)
								.set_minimum_version(1, 3)
								.set_surface(surface)
								.select();

	if(!device_selector_ret) {
		spdlog::error("Failed to create Vulkan Device Selector {}", device_selector_ret.error().message());
		return false;
	}

	VkPhysicalDeviceShaderDrawParametersFeatures draw_features = {};
	draw_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
	draw_features.shaderDrawParameters = VK_TRUE;

	VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering = {};
	dynamic_rendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
	dynamic_rendering.dynamicRendering = VK_TRUE;

	vkb::DeviceBuilder device_builder { device_selector_ret.value() };
	auto device_builder_ret = device_builder
								.add_pNext(&draw_features)
								.add_pNext(&dynamic_rendering)
								.build();

	if(!device_builder_ret) {
		spdlog::error("Failed to create Vulkan Device {}", device_builder_ret.error().message());
		return false;
	}

	device = device_builder_ret.value(); // ze bluechowth dewice has connectedey suchessfulley

	spdlog::info("Found capable render device: {}", device.physical_device.name);

	deletion_queue.push_back([=, this]() {
		vkb::destroy_device(device);
	});

	vkCmdBeginRenderingKHR = reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(vkGetDeviceProcAddr(device, "vkCmdBeginRenderingKHR"));
	vkCmdEndRenderingKHR = reinterpret_cast<PFN_vkCmdEndRenderingKHR>(vkGetDeviceProcAddr(device, "vkCmdEndRenderingKHR"));

	return true;
}

bool Renderer::create_swapchain() {
	VkSurfaceFormatKHR image_format = {};
	image_format.format = VK_FORMAT_R8G8B8A8_UNORM;
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

	deletion_queue.push_back([=, this]() {
		swapchain.destroy_image_views(swapchain_image_views);
		vkb::destroy_swapchain(swapchain);
	});

	return true;
}

bool Renderer::create_queues() {
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

bool Renderer::create_pipeline_cache() {
	VkPipelineCacheCreateInfo pipeline_cache_create_info  = {};
	pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	// load cache data
	/*
	{
		std::vector<char> pipeline_cache_data = fs::read_local<char>("pipeline_cache_data.bin", true);
		char *cache_data = pipeline_cache_data.data();
		size_t cache_size = pipeline_cache_data.size();

		bool bad_cache = false;

		uint32_t header_length = 0;
		uint32_t header_version = 0;
		uint32_t vendor_id = 0;
		uint32_t device_id = 0;
		uint32_t uuid[VK_UUID_SIZE] = {};

		memcpy(&header_length, (uint8_t*)cache_data, 4);
		memcpy(&header_version, (uint8_t*)cache_data + 4, 4);
		memcpy(&vendor_id, (uint8_t*)cache_data + 8, 4);
		memcpy(&device_id, (uint8_t*)cache_data + 12, 4);
		memcpy(&uuid, (uint8_t*)cache_data + 16, VK_UUID_SIZE);

		if(header_length <= 0) bad_cache = true;
		if(header_version != VK_PIPELINE_CACHE_HEADER_VERSION_ONE) bad_cache = true;
		if(vendor_id != device.physical_device.properties.vendorID) bad_cache = true;
		if(device_id != device.physical_device.properties.deviceID) bad_cache = true;
		if(memcmp(uuid, device.physical_device.properties.pipelineCacheUUID, VK_UUID_SIZE) != 0) bad_cache = true;

		if(!bad_cache) {
			pipeline_cache_create_info.initialDataSize = static_cast<uint32_t>(cache_size);
			pipeline_cache_create_info.pInitialData = pipeline_cache_data.data();
		}
	}
	*/

	VK_CHECK_BOOL(vkCreatePipelineCache(device, &pipeline_cache_create_info, nullptr, &pipeline_cache));

	deletion_queue.push_back([=, this]() {
		vkDestroyPipelineCache(device, pipeline_cache, nullptr);
	});

	return true;
}


bool Renderer::create_descriptor_layout() {
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings = {
			info::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
		};

		auto layout_info = info::descriptor_set_layout_info(bindings);
		VK_CHECK_BOOL(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &global_descriptor_layout));
	}

	{
		std::vector<VkDescriptorSetLayoutBinding> bindings = {
			info::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0)
		};

		auto layout_info = info::descriptor_set_layout_info(bindings);
		VK_CHECK_BOOL(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &object_descriptor_layout));
	}

	{
		std::vector<VkDescriptorSetLayoutBinding> bindings = {
			info::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
		};

		auto layout_info = info::descriptor_set_layout_info(bindings);
		VK_CHECK_BOOL(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &sample_descriptor_layout));
	}

	deletion_queue.push_back([=, this]() {
		vkDestroyDescriptorSetLayout(device, object_descriptor_layout, nullptr);
		vkDestroyDescriptorSetLayout(device, global_descriptor_layout, nullptr);
		vkDestroyDescriptorSetLayout(device, sample_descriptor_layout, nullptr);
	});

	return true;
}

bool Renderer::create_pipelines() {
	if(!build_vertex_layout()) return false;
	if(!build_vertex_pipelines()) return false;

	{
		size_t size = 0;

		VK_CHECK_BOOL(vkGetPipelineCacheData(device, pipeline_cache, &size, nullptr));

		std::vector<char> pipeline_data(size);
		VK_CHECK_BOOL(vkGetPipelineCacheData(device, pipeline_cache, &size, static_cast<void*>(pipeline_data.data())));

		std::ofstream file("pipeline_cache_data.bin", std::ofstream::out | std::ofstream::binary);

		std::copy(pipeline_data.cbegin(), pipeline_data.cend(), std::ostream_iterator<char>(file));

		file.close();
	}

	deletion_queue.push_back([=, this] {
		for(auto &map: pipelines) {
			vkDestroyPipeline(device, map.second, nullptr);
		}

		for(auto &map: pipeline_layouts) {
			vkDestroyPipelineLayout(device, map.second, nullptr);
		}
	});

	return true;
}

bool Renderer::create_vma_allocator() {
	VmaAllocatorCreateInfo allocator_info = {};
	allocator_info.physicalDevice = device.physical_device;
	allocator_info.device = device;
	allocator_info.instance = instance;

	VK_CHECK_BOOL(vmaCreateAllocator(&allocator_info, &allocator));

	deletion_queue.push_back([=, this]() {
		vmaDestroyAllocator(allocator);
	});

	return true;
}

bool Renderer::create_depth_image() {
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
	view_info.image = depth_image.image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = depth_format;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	VK_CHECK_BOOL(vkCreateImageView(device, &view_info, nullptr, &depth_image.view));

	deletion_queue.push_back([=, this]() {
		vkDestroyImageView(device, depth_image.view, nullptr);
		vmaDestroyImage(allocator, depth_image.image, depth_image.allocation);
	});

	return true;
}

bool Renderer::create_texture_sampler() {
	VkSamplerCreateInfo sampler_info = {};
	return true;
}

bool Renderer::create_texture_array() {
	return true;
}

bool Renderer::create_descriptor_pool() {
	{
		std::vector<VkDescriptorPoolSize> pool_sizes = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 10 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();
		pool_info.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 3;

		VK_CHECK_BOOL(vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool));
	}

	deletion_queue.push_back([=, this]() {
		vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
	});

	return true;
}

bool Renderer::create_uniform_buffers() {
	camera_data_buffers.resize(MAX_FRAMES_IN_FLIGHT);
	object_data_buffers.resize(MAX_FRAMES_IN_FLIGHT);

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		{
			auto buffer_info = info::buffer_create_info(sizeof(EGlobalData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
			auto allocate_info = info::allocation_create_info(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
																VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			VK_CHECK_BOOL(camera_data_buffers[i].memory.allocate(allocator, &buffer_info, &allocate_info, &camera_data_buffers[i].info));
		}

		{
			auto buffer_info = info::buffer_create_info(sizeof(EObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			auto allocate_info = info::allocation_create_info(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
																VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			VK_CHECK_BOOL(object_data_buffers[i].memory.allocate(allocator, &buffer_info, &allocate_info, &object_data_buffers[i].info))
		}

		deletion_queue.push_back([=, this]() {
			object_data_buffers[i].memory.destroy();
			camera_data_buffers[i].memory.destroy();
		});
	}

	return true;
}

bool Renderer::create_descriptor_sets() {
	global_descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);
	object_descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);
	sample_descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);

	std::vector<VkDescriptorSetLayout> camera_layouts(MAX_FRAMES_IN_FLIGHT, global_descriptor_layout);
	{
		VkDescriptorSetAllocateInfo camera_descriptor_set_info = info::descriptor_set_allocate_info(camera_layouts, descriptor_pool);
		VK_CHECK_BOOL(vkAllocateDescriptorSets(device, &camera_descriptor_set_info, global_descriptor_sets.data()));
	}

	std::vector<VkDescriptorSetLayout> object_layouts(MAX_FRAMES_IN_FLIGHT, object_descriptor_layout);
	{
		auto object_descriptor_set_info = info::descriptor_set_allocate_info(object_layouts, descriptor_pool);
		VK_CHECK_BOOL(vkAllocateDescriptorSets(device, &object_descriptor_set_info, object_descriptor_sets.data()));
	}

	//std::vector<VkDescriptorSetLayout> sample_layouts(MAX_FRAMES_IN_FLIGHT, sample_descriptor_layout);
	//{
	//	auto sample_descriptor_set_info = info::descriptor_set_allocate_info(object_layouts, descriptor_pool);
	//	VK_CHECK_BOOL(vkAllocateDescriptorSets(device, &sample_descriptor_set_info, sample_descriptor_sets.data()));
	//}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		// write global sets
		VkDescriptorBufferInfo buffer_info = {};
		buffer_info.buffer = camera_data_buffers[i].memory.buffer;
		buffer_info.offset = 0;
		buffer_info.range = sizeof(EGlobalData);

		VkWriteDescriptorSet global_descriptor_write = {};
		global_descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		global_descriptor_write.dstSet = global_descriptor_sets[i];
		global_descriptor_write.dstBinding = 0;
		global_descriptor_write.dstArrayElement = 0;
		global_descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		global_descriptor_write.descriptorCount = 1;
		global_descriptor_write.pBufferInfo = &buffer_info;

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

		VkWriteDescriptorSet descriptor_writes[] = { global_descriptor_write, object_descriptor_write };

		vkUpdateDescriptorSets(device, 2, descriptor_writes, 0, nullptr);
	}

	return true;
}

bool Renderer::create_command_pool() {
	command_buffers.resize(swapchain_images.size());

	auto command_pool_info = info::command_pool_create_info(graphics_queue_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	VK_CHECK_BOOL(vkCreateCommandPool(device, &command_pool_info, nullptr, &command_pool));

	auto command_allocate_info = info::command_buffer_allocate_info(command_pool, static_cast<uint32_t>(command_buffers.size()));
	VK_CHECK_BOOL(vkAllocateCommandBuffers(device, &command_allocate_info, command_buffers.data()));

	deletion_queue.push_back([=, this]() {
		vkDestroyCommandPool(device, command_pool, nullptr);
	});

	return true;
}
bool Renderer::create_sync_objects() {
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

	deletion_queue.push_back([=, this]() {
		for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, finished_semaphores[i], nullptr);
			vkDestroySemaphore(device, available_semaphores[i], nullptr);
			vkDestroyFence(device, in_flight_fences[i], nullptr);
		}
	});

	return true;
}

bool Renderer::create_imgui() {
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
	init_info.ImageCount = (uint32_t)swapchain_images.size();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr;
	init_info.UseDynamicRendering = true;
	init_info.ColorAttachmentFormat = swapchain.image_format;
	init_info.DepthAttachmentFormat = find_depth_format();

	ImGui_ImplVulkan_Init(&init_info, VK_NULL_HANDLE);

	io.FontDefault = io.Fonts->AddFontFromFileTTF("./assets/fonts/Roboto-Regular.ttf", 16);

	submit_command([=, this](VkCommandBuffer command) {
		ImGui_ImplVulkan_CreateFontsTexture(command);
	});

	vkDeviceWaitIdle(device);

	ImGui_ImplVulkan_DestroyFontUploadObjects();

	deletion_queue.push_back([=, this]() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
	});

	return true;
}

bool Renderer::rebuild_swapchain() {
	spdlog::debug("Rebuilding swapchain");

	vkDeviceWaitIdle(device);

	vkDestroyCommandPool(device, command_pool, nullptr);

	vkDestroyImageView(device, depth_image.view, nullptr);
	vmaDestroyImage(allocator, depth_image.image, depth_image.allocation);

	swapchain.destroy_image_views(swapchain_image_views);

	if(!create_swapchain()) 		return false;
	if(!create_depth_image())		return false;
	if(!create_command_pool()) 		return false;

	// HACK: above create commands push a deleter onto the queue, we need to pop them so they don't double free
	for(size_t i = 0; i <= 2; i++)
		deletion_queue.pop_back();

	return true;
}

VkFormat Renderer::find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
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

VkFormat Renderer::find_depth_format() {
	return find_supported_format({
		VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
	}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void Renderer::submit_command(std::function<void(VkCommandBuffer command)> &&function) {
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

bool Renderer::build_vertex_layout() {
	std::vector<VkDescriptorSetLayout> descriptor_layouts = {
		global_descriptor_layout,
		object_descriptor_layout,
		sample_descriptor_layout
	};

	auto pipeline_layout_info = info::pipeline_layout_info(descriptor_layouts);
	VK_CHECK_BOOL(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layouts["vertex"]));

	return true;
}

bool Renderer::build_vertex_pipelines() {
	RenderPipelineConstructor pipeline_constructor(device, pipeline_layouts["vertex"], pipeline_cache, VK_NULL_HANDLE);

	auto binding_descriptions = EVertex::get_binding_descriptions();
	auto attribute_descriptions = EVertex::get_attribute_descriptions();
	pipeline_constructor.input_info = info::input_vertex_info(binding_descriptions, attribute_descriptions);

	pipeline_constructor.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipeline_constructor.input_assembly.primitiveRestartEnable = VK_FALSE;

	VkShaderModule vertex_vert = create_shader(fs::read_asset<uint32_t>("shaders/vertex.vert.spv", true));
	VkShaderModule vertex_frag = create_shader(fs::read_asset<uint32_t>("shaders/vertex.frag.spv", true));

	pipeline_constructor.add_shader(VK_SHADER_STAGE_VERTEX_BIT, vertex_vert);
	pipeline_constructor.add_shader(VK_SHADER_STAGE_FRAGMENT_BIT, vertex_frag);

	pipeline_constructor.viewport_state.viewportCount = 1;
	pipeline_constructor.viewport_state.scissorCount = 1;

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

	// create data type for dynamic rendering
	std::vector<VkFormat> color_attachment_formats = {
		swapchain.image_format
	};
	auto pipeline_rendering_info = info::rendering_create_info(color_attachment_formats, find_depth_format());
	pipeline_constructor.pipeline_info.pNext = &pipeline_rendering_info;

	VK_CHECK_BOOL(pipeline_constructor.build(&pipelines["vertex"]));
	pipeline_constructor.rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
	VK_CHECK_BOOL(pipeline_constructor.build(&pipelines["vertex_wireframe"]));
	pipeline_constructor.rasterizer.polygonMode = VK_POLYGON_MODE_POINT;
	VK_CHECK_BOOL(pipeline_constructor.build(&pipelines["vertex_points"]));

	// reset to fill mode for following pipelines...
	pipeline_constructor.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

	vkDestroyShaderModule(device, vertex_vert, nullptr);
	vkDestroyShaderModule(device, vertex_frag, nullptr);

	VkShaderModule phong_vert = create_shader(fs::read_asset<uint32_t>("shaders/phong.vert.spv", true));
	VkShaderModule phong_frag = create_shader(fs::read_asset<uint32_t>("shaders/phong.frag.spv", true));

	pipeline_constructor.shader_stages.clear();
	pipeline_constructor.add_shader(VK_SHADER_STAGE_VERTEX_BIT, phong_vert);
	pipeline_constructor.add_shader(VK_SHADER_STAGE_FRAGMENT_BIT, phong_frag);

	VK_CHECK_BOOL(pipeline_constructor.build(&pipelines["vertex_phong"]));

	vkDestroyShaderModule(device, phong_vert, nullptr);
	vkDestroyShaderModule(device, phong_frag, nullptr);

	VkShaderModule toon_vert = create_shader(fs::read_asset<uint32_t>("shaders/toon.vert.spv", true));
	VkShaderModule toon_frag = create_shader(fs::read_asset<uint32_t>("shaders/toon.frag.spv", true));

	pipeline_constructor.shader_stages.clear();
	pipeline_constructor.add_shader(VK_SHADER_STAGE_VERTEX_BIT, toon_vert);
	pipeline_constructor.add_shader(VK_SHADER_STAGE_FRAGMENT_BIT, toon_frag);

	VK_CHECK_BOOL(pipeline_constructor.build(&pipelines["vertex_toon"]));

	vkDestroyShaderModule(device, toon_vert, nullptr);
	vkDestroyShaderModule(device, toon_frag, nullptr);

	return true;
}
