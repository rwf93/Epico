#pragma once

class VulkanInstance;
class VulkanSurface;
class VulkanDevice {
public:
	VulkanDevice() = default;
	~VulkanDevice() { vkb::destroy_device(device); };

	void init(VulkanInstance *instance, VulkanSurface *surface);

	void wait() { vkDeviceWaitIdle(device); };

	vkb::Device &get_device() { return device; }
	VkQueue &get_graphics_queue() { return graphics_queue; }
	VkQueue &get_present_queue() { return present_queue; }
	uint32_t get_graphics_queue_index() { return graphics_queue_index; }
protected:
	void retreive_device();
	void retreive_queues();
private:
	VulkanInstance *instance = nullptr;
	VulkanSurface *surface = nullptr;

	vkb::Device device = {};

	VkQueue graphics_queue = VK_NULL_HANDLE;
	VkQueue present_queue = VK_NULL_HANDLE;

	uint32_t graphics_queue_index = 0;
};