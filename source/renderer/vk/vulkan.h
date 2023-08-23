#pragma once

#define FUNC_CREATE_SHADER const std::vector<char> &code

namespace render {

class VulkanRenderer {
public:
    VulkanRenderer(GameGlobals *game);
    ~VulkanRenderer();

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
    bool create_descriptor_pool();

    bool setup_imgui();

    bool rebuild_swapchain();
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

    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;

    const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    std::vector<VkSemaphore> available_semaphores;
    std::vector<VkSemaphore> finished_semaphores;
    std::vector<VkFence> in_flight_fences;

    size_t current_frame = 0;
private:
    RenderPipelineConstructor pipeline_constructor;
    VkPipelineCache pipeline_cache;

    VkPipeline triangle_pipeline;

private:
    struct {
        VkBuffer buffer;
        VmaAllocation allocation;
    } VertexBuffer;

    struct {
        VkBuffer buffer;
        VmaAllocation allocation;
        uint32_t count;
    } IndexBuffer;

    struct {
        glm::vec2 pos;
        glm::vec3 color;

        static VkVertexInputBindingDescription get_binding_description() {
            VkVertexInputBindingDescription binding_description = {};

            binding_description.binding = 0;
            binding_description.stride = sizeof(Vertex);
            binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return binding_description;
        }

        static std::array<VkVertexInputAttributeDescription, 2> get_attribute_descriptions() {
            std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions = {};

            attribute_descriptions[0].binding = 0;
            attribute_descriptions[0].location = 0;
            attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attribute_descriptions[0].offset = offsetof(Vertex, pos);

            return attribute_descriptions;
        }
    } Vertex;

    VkDescriptorPool descriptor_pool;
};

}