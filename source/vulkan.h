#pragma once

#define FUNC_CREATE_SHADER const std::vector<char> &code

class vulkanRenderer {
public:
    vulkanRenderer(gameGlobals *game);
    ~vulkanRenderer();

    bool create_vk_instance();
    bool create_surface();
    bool create_device();
    bool create_swapchain();
    bool create_queues();

    bool setup();

    VkShaderModule create_shader(FUNC_CREATE_SHADER);

private:
    gameGlobals *game;

    vkb::Instance instance;
	VkSurfaceKHR surface;
	vkb::Device device;

    vkb::Swapchain swapchain;
    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;

    VkQueue graphics_queue;
    uint32_t graphics_queue_family;

};

class renderPassConstructor {

};

class renderPipelineConstructor {

};