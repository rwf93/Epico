#pragma once

struct VulkanFrameContext {
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;

    VkSemaphore available_semaphore;
    VkSemaphore finished_semaphore;
    VkFence fence;
};

class VulkanDevice;
class VulkanSwapchain;
class VulkanCommandPool {
public:
    VulkanCommandPool(VulkanDevice *device, VulkanSwapchain *swapchain);
    ~VulkanCommandPool();
    // basically same as the destructor
    void rebuild();

    void wait_fences();
    void reset_fences();

    void begin_recording();
    void end_recording();

    VulkanFrameContext &get_frame_context(uint32_t index) { return frame_contexts[index]; }
    VulkanFrameContext &get_frame_context() { return get_frame_context(current_frame); }

    VkCommandBuffer &get_command(uint32_t index) { return get_frame_context(index).command_buffer; }
    VkCommandBuffer &get_command() { return get_command(current_frame); };

    VkSemaphore &get_available_semaphore(uint32_t index) { return get_frame_context(index).available_semaphore; }
    VkSemaphore &get_available_semaphore() { return get_available_semaphore(current_frame); }

    VkSemaphore &get_finished_semaphore(uint32_t index) { return get_frame_context(index).finished_semaphore; }
    VkSemaphore &get_finished_semaphore() { return get_available_semaphore(current_frame); }

    VkFence &get_fence(uint32_t index) { return get_frame_context(index).fence; }
    VkFence &get_fence() { return get_fence(current_frame); }

    uint32_t get_max_flying_frames() { return max_flying_frames; }

    // Flips the frame for the backbuffer.
    void advance() { current_frame = (current_frame + 1) % max_flying_frames; }

    // onetime command submitting
    using SubmitCommandFunction = std::function<void(VkCommandBuffer command)>;
    void submit_command(SubmitCommandFunction &&command_function);
private:
    void create_command_pool();
    void create_sync_objects();
private:
    VulkanDevice *device = nullptr;
    VulkanSwapchain *swapchain = nullptr;

    std::vector<VulkanFrameContext> frame_contexts;

    uint32_t max_flying_frames = 0;
    uint32_t current_frame = 0;
};