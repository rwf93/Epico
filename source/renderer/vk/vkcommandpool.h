#pragma once

class VulkanDevice;
class VulkanSwapchain;
class VulkanCommandPool {
public:
    VulkanCommandPool(VulkanDevice *device, VulkanSwapchain *swapchain);
    ~VulkanCommandPool();
    // basically same as the destructor
    void rebuild();

    void begin_recording();
    void end_recording();

    VkCommandBuffer &get_command(uint32_t index) { return command_buffers[index]; }
    VkCommandBuffer &get_command() { return get_command(current_frame); };

    using SubmitCommandFunction = std::function<void(VkCommandBuffer command)>;
    static void submit_command(SubmitCommandFunction &&command_function);
private:
    VulkanDevice *device = nullptr;
    VulkanSwapchain *swapchain = nullptr;

    VkCommandPool command_pool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> command_buffers = {};

    uint32_t max_flying_frames = 0;
    uint32_t image_index = 0;
    uint32_t current_frame = 0;
};