#pragma once

#include <vk_mem_alloc.h>

#include "primitives.h"

namespace render {

class VulkanRenderer {
public:
    VulkanRenderer(GameGlobals *game);
    ~VulkanRenderer();

    bool setup();
    bool draw();

    VkShaderModule create_shader(const std::vector<uint32_t> &code);

private:
    bool create_vk_instance();
    bool create_surface();
    bool create_device();
    bool create_swapchain();
    bool create_queues();
    bool create_render_pass();
    bool create_pipeline_cache();
    bool create_descriptor_layout();
    bool create_pipelines();
    bool create_vma_allocator();
    bool create_depth_image();
    bool create_framebuffers();
    bool create_descriptor_pool();
    bool create_uniform_buffers();
    bool create_descriptor_sets();
    bool create_command_pool();
    bool create_sync_objects();

    bool create_imgui();

    bool rebuild_swapchain();

    VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat find_depth_format();
private:
    GameGlobals *game;

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

    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorSetLayout descriptor_set_layout_dynamic;

    // funny pipelining
    PipelinePair build_triangle_pipeline();
    PipelinePair build_vertex_pipeline();
    PipelinePair build_imgui_pipeline();

    VkPipelineCache pipeline_cache;
    std::map<std::string, PipelinePair> pipelines;

    VmaAllocator allocator;

    EImage depth_image;

    struct UniformBufferAllocation {
        EBuffer memory;
        VmaAllocationInfo info;
    };

    VkDescriptorPool descriptor_pool;
    VkDescriptorPool imgui_descriptor_pool;

    const uint32_t MAX_OBJECTS = 10000;

    std::vector<UniformBufferAllocation> camera_data_buffers;
    std::vector<VkDescriptorSet> descriptor_sets;

    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;

    const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    std::vector<VkSemaphore> available_semaphores;
    std::vector<VkSemaphore> finished_semaphores;
    std::vector<VkFence> in_flight_fences;

    size_t current_frame = 0;

    std::deque<std::function<void()>> deletion_queue;
};

}