#pragma once

class VulkanDevice;
class VulkanCommandPool;
class VulkanSwapchain {
public:
    VulkanSwapchain();
    ~VulkanSwapchain();

    void init(FunctorQueue<> &queue, VulkanDevice *vkdevice);
    void fini();

    void rebuild() { create_swapchain(true); }

    vkb::Swapchain &get_swapchain() { return swapchain; }
    std::vector<VkImage> &get_swapchain_images() { return swapchain_images; }
    std::vector<VkImageView> &get_swapchain_image_views() { return swapchain_image_views; }

    VkImage &get_swapchain_image(uint32_t index) { return get_swapchain_images().at(index); }
    VkImage &get_swapchain_image() { return get_swapchain_image(image_index); }

    VkImageView &get_swapchain_image_view(uint32_t index) { return get_swapchain_image_views().at(index); }
    VkImageView &get_swapchain_image_view() { return get_swapchain_image_view(image_index); }

    void transition_image(VkCommandBuffer command, VkImage image, VkImageLayout current_layout, VkImageLayout new_layout);

    bool aquire_next_image(VulkanCommandPool *command_pool);

    uint32_t &get_image_index() { return image_index; }
private:
    void create_swapchain(bool rebuild = false);
private:
    VulkanDevice *device = nullptr;

    vkb::Swapchain swapchain = {};
    std::vector<VkImage> swapchain_images = {};
	std::vector<VkImageView> swapchain_image_views = {};

    uint32_t image_index = 0;
};