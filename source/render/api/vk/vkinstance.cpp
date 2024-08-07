#include "vkinstance.h"


static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT serverity,
	VkDebugUtilsMessageTypeFlagsEXT message_types,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void *user_data
) {
	UNUSED(serverity);
	UNUSED(message_types);
	UNUSED(user_data);
	UNUSED(callback_data)

	switch(serverity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: 		LOGGER->info(callback_data->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: 	LOGGER->warn(callback_data->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: 	LOGGER->error(callback_data->pMessage); break;
	default: break;
	}

	return VK_FALSE;
}

void VulkanInstance::init(FunctorQueue<> &queue) {
	vkb::InstanceBuilder builder;
	auto builder_ret = builder
						.set_app_name("Epico")
						.set_engine_name("Epico Engine")
						.require_api_version(VK_API_VERSION_1_3)
						//.request_validation_layers()
						.set_debug_callback(vk_debug_callback)
						.enable_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)
						.build();

	if(!builder_ret.has_value()) {
		LOGGER->error("Failed to create Vulkan Instance: {}", builder_ret.error().message());
		std::abort();
	}

	instance = builder_ret.value();

	volkLoadInstance(instance);

	queue.push([&] { fini(); });
}

void VulkanInstance::fini() {
	vkb::destroy_instance(instance);
}

