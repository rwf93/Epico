#pragma once

#define FUNC_CREATE_SHADER const std::vector<char> &code

class vulkanRenderer {
public:
    vulkanRenderer(gameGlobals *game);
    ~vulkanRenderer();

    bool setup();
    bool draw();

    VkShaderModule create_shader(FUNC_CREATE_SHADER);

private:
    bool create_vk_instance();
    bool create_surface();
    bool create_device();
    bool create_swapchain();
    bool create_queues();
    bool create_render_pass();
    bool create_pipeline_cache();
    bool create_pipelines();
    bool create_framebuffers();
    bool create_command_pool();
    bool create_sync_objects();
    bool create_imgui();

    bool rebuild_swapchain();
private:
    renderPipelineConstructor triangle_pipeline;
    VkPipelineCache pipeline_cache;

private:
    gameGlobals *game;

    vkb::Instance instance;
	VkSurfaceKHR surface;
	vkb::Device device;

    vkb::Swapchain swapchain;
    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;

    VkQueue graphics_queue;
    VkQueue present_queue;
    uint32_t graphics_queue_index;

    VkRenderPass render_pass;

    std::vector<VkFramebuffer> framebuffers;

    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;

    const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    std::vector<VkSemaphore> available_semaphores;
    std::vector<VkSemaphore> finished_semaphores;
    std::vector<VkFence> in_flight_fences;

    size_t current_frame = 0;
};