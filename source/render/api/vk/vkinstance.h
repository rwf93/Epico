#pragma once

class VulkanInstance {
public:
	VulkanInstance();
	~VulkanInstance() { vkb::destroy_instance(instance); };

	static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT serverity,
		VkDebugUtilsMessageTypeFlagsEXT message_types,
		const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
		void *user_data
	);

	vkb::Instance &get_instance() { return instance; }
private:
	vkb::Instance instance = {};
};