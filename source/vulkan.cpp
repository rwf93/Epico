#include "vulkan.h"

vulkanRenderer::vulkanRenderer(gameGlobals *game) {
	this->game = game;
}

vulkanRenderer::~vulkanRenderer() {

	for(auto image_view: swapchain_image_views)
		vkDestroyImageView(device, image_view, nullptr);

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
									.build();

	if(!instance_builder_ret) {
		fmt::println("Failed to create Vulkan instance {}", instance_builder_ret.error().message());
		return false;
	}

	instance = instance_builder_ret.value();

	return true;
}

bool vulkanRenderer::create_surface() {
	if (!SDL_Vulkan_CreateSurface(game->window, instance.instance, &surface)) {
		fmt::println("Failed to create SDL Surface {}", SDL_GetError());
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
		fmt::println("Failed to create Vulkan Device Selector {}", device_selector_ret.error().message());
		return false;
	}

	vkb::DeviceBuilder device_builder{ device_selector_ret.value() };
	auto device_builder_ret = device_builder.build();

	if(!device_builder_ret) {
		fmt::println("Failed to create Vulkan Device {}", device_builder_ret.error().message());
		return false;
	}

	device = device_builder_ret.value(); // ze bluechowth dewice has connectedey suchessfulley

	return true;
}

bool vulkanRenderer::create_swapchain() {
	vkb::SwapchainBuilder swapchain_builder{ device };
	auto swapchain_builder_ret = swapchain_builder.build();

	if(!swapchain_builder_ret) {
		fmt::println("Failed to create Vulkan Swapchain {}", swapchain_builder_ret.error().message());
		return false;
	}

	swapchain = swapchain_builder_ret.value();
	swapchain_image_views = swapchain.get_image_views().value();
	swapchain_images = swapchain.get_images().value();

	return true;
}

bool vulkanRenderer::create_queues() {
	auto gq = device.get_queue(vkb::QueueType::graphics);
	auto gqi = device.get_queue_index(vkb::QueueType::graphics);

	if (!gq.has_value()) {
		fmt::println("Failed to create Get Vulkan Graphics Queue");
		return false;
	}

	if (!gqi.has_value()) {
		fmt::println("Failed to create Get Vulkan Graphics Queue Index");
		return false;
	}

	graphics_queue = gq.value();
	graphics_queue_family = gqi.value();

	return true;
}

bool vulkanRenderer::setup() {
	if(!create_vk_instance()) 		return false;
	if(!create_surface()) 			return false;
	if(!create_device()) 			return false;
	if(!create_swapchain()) 		return false;
	if(!create_queues())			return false;

	return true;
}

VkShaderModule vulkanRenderer::create_shader(FUNC_CREATE_SHADER) {
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = code.size();
	create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shader_module;
	if(vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
		fmt::println("Warning: couldn't create shader module");
		return VK_NULL_HANDLE;
	}

	return shader_module;
}