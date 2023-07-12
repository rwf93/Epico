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
    bool create_framebuffers();
    bool create_command_pool();
    bool create_sync_objects();

private:
    bool create_test_pipeline();

private:
    VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;

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
    std::vector<VkFence> images_in_flight;

    uint64_t image_index;

    size_t current_frame = 0;
};

// 4l8r
class renderPipelineConstructor {
    VkPipeline build(VkDevice device, VkRenderPass pass);
private:
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineVertexInputStateCreateInfo vertex_input_info;
    VkPipelineInputAssemblyStateCreateInfo input_assembly;
    VkViewport view_port;
    VkRect2D scissor;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineColorBlendAttachmentState color_blend_attachment;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineLayout pipelineLayout;
};