#include "vkdevice.h"
#include "vkinstance.h"
#include "vksurface.h"

void VulkanDevice::init(FunctorQueue<> &queue, VulkanInstance *vkinstance, VulkanSurface *vksurface) {
	this->instance = vkinstance;
	this->surface = vksurface;

	retreive_device();
	retreive_queues();

	queue.push([&] { fini(); });
}

void VulkanDevice::fini() {
	vkb::destroy_device(device);
}

void VulkanDevice::retreive_device() {
	VkPhysicalDeviceVulkan13Features features_13 = {};
	features_13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	features_13.dynamicRendering = true;
	features_13.synchronization2 = true;

	VkPhysicalDeviceVulkan12Features features_12 = {};
	features_12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features_12.bufferDeviceAddress = true;
	features_12.descriptorIndexing = true;

	vkb::PhysicalDeviceSelector selector(instance->get_instance());
	auto selector_ret = selector
							.set_surface(surface->get_surface())
							.set_minimum_version(1, 3)
							.set_required_features_13(features_13)
							.set_required_features_12(features_12)
							.add_required_extension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME)
							.select();

	if(!selector_ret.has_value()) {
		LOGGER->error("Couldn't create Physical Device Selector", selector_ret.error().message());
		std::abort();
	}

	vkb::DeviceBuilder builder(selector_ret.value());
	auto builder_ret = builder
		.build();

	if(!builder_ret.has_value()) {
		LOGGER->error("Couldn't create Device Builder", builder_ret.error().message());
		std::abort();
	}

	this->device = builder_ret.value(); // ze vuwlkan dewice is weady to wender. :DDDDD

	volkLoadDevice(this->device);

	LOGGER->info("{}: {}",
		rand() % 15 == 1
			? "ze vuwlkan dewice is weady to wender"
			: "Found capable render device",
		device.physical_device.name
	);
}

void VulkanDevice::retreive_queues() {
	// Yes, I am acutely aware the naming scheme here is bad.
	auto gq = device.get_queue(vkb::QueueType::graphics);
	auto pq = device.get_queue(vkb::QueueType::present);
	auto gqi = device.get_queue_index(vkb::QueueType::graphics);

	if (!gq.has_value()) {
		LOGGER->error("Failed to get Vulkan Graphics Queue");
		std::abort();
	}

	if(!pq.has_value()) {
		LOGGER->error("Failed to get Vulkan Present Queue");
		std::abort();
	}

	if (!gqi.has_value()) {
		LOGGER->error("Failed to get Vulkan Graphics Queue Index");
		std::abort();
	}

	graphics_queue = gq.value();
	present_queue = pq.value();
	graphics_queue_index = gqi.value();
}