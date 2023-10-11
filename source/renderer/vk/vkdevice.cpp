#include "vkdevice.h"
#include "vkinstance.h"
#include "vksurface.h"

VulkanDevice::VulkanDevice(VulkanInstance *instance, VulkanSurface *surface) {
    this->instance = instance;
    this->surface = surface;

    vkb::PhysicalDeviceSelector selector(instance->get_instance());
    auto selector_ret = selector
                            .set_surface(surface->get_surface())
                            .set_minimum_version(1, 3)
                            .select();

    if(!selector_ret.has_value()) {
        spdlog::error("Couldn't create Physical Device Selector", selector_ret.error().message());
        std::abort();
    }

    vkb::DeviceBuilder builder(selector_ret.value());
    auto builder_ret = builder
                        .build();

    if(!builder_ret.has_value()) {
        spdlog::error("Couldn't create Device Builder", builder_ret.error().message());
        std::abort();
    }

    device = builder_ret.value();

    spdlog::info("Found capable render device: {}", device.physical_device.name);

    auto gq = device.get_queue(vkb::QueueType::graphics);
	auto pq = device.get_queue(vkb::QueueType::present);
	auto gqi = device.get_queue_index(vkb::QueueType::graphics);

	if (!gq.has_value()) {
		spdlog::error("Failed to get Vulkan Graphics Queue");
        std::abort();
	}

	if(!pq.has_value()) {
		spdlog::error("Failed to get Vulkan Present Queue");
        std::abort();
	}

	if (!gqi.has_value()) {
		spdlog::error("Failed to get Vulkan Graphics Queue Index");
        std::abort();
	}

	graphics_queue = gq.value();
	present_queue = pq.value();
	graphics_queue_index = gqi.value();
}

VulkanDevice::~VulkanDevice() {
    vkb::destroy_device(device);
}