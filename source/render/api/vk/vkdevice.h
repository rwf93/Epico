#pragma once

class VulkanInstance;
class VulkanSurface;
class VulkanDevice {
public:
	VulkanDevice(VulkanInstance *instance, VulkanSurface *surface);
	~VulkanDevice() { vkb::destroy_device(device); };

	void wait() { vkDeviceWaitIdle(device); };

	vkb::Device &get_device() { return device; }
	VkQueue &get_graphics_queue() { return graphics_queue; }
	VkQueue &get_present_queue() { return present_queue; }
	uint32_t get_graphics_queue_index() { return graphics_queue_index; }

	std::optional<VkFormat> find_supported_format(
		const std::vector<VkFormat> &candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features
	) {
		for(VkFormat format: candidates) {
			VkFormatProperties properties = {};
			vkGetPhysicalDeviceFormatProperties(get_device().physical_device, format, &properties);

			if(tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
				return format;
			}

			if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		return std::nullopt;
	}

	std::optional<VkFormat> find_depth_format() {
		return find_supported_format(
			{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}
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