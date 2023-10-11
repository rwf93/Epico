#pragma once

class VulkanDevice;
class VulkanSwapchain {
public:
    VulkanSwapchain(VulkanDevice *device);
    ~VulkanSwapchain();

    void rebuild();

    vkb::Swapchain &get_swapchain() { return swapchain; }
    std::vector<VkImage> &get_swapchain_images() { return swapchain_images; }
    std::vector<VkImageView> &get_swapchain_image_views() { return swapchain_image_views; }
private:
    VulkanDevice *device = nullptr;

    vkb::Swapchain swapchain = {};
    std::vector<VkImage> swapchain_images = {};
	std::vector<VkImageView> swapchain_image_views = {};
};