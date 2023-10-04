#pragma once

#include <vk_mem_alloc.h>

#include "tools.h"

#include "globals.h"
#include "primitives.h"
#include "buffer.h"
#include "image.h"
#include "mesh.h"

namespace render {

class Renderer {
public:
	Renderer(GameGlobals *game);
	~Renderer();

	bool setup();
	bool begin();
	bool end();

	VmaAllocator get_allocator() { return allocator; }

	void submit_command(std::function<void(VkCommandBuffer command)> &&function);
private:
	bool create_instance();
	bool create_surface();
	bool create_device();
	bool create_queues();
	bool create_swapchain(bool rebuild = false);
	bool create_command_pool(bool rebuild = false);
	bool create_sync_objects();

	bool create_vma_allocator();
	bool create_depth_image(bool rebuild = false);

	bool create_descriptor_layout();
	bool create_descriptor_pool();
	bool create_uniform_buffers();
	bool create_descriptor_sets();

	bool create_pipeline_cache();
	bool create_imgui();
	bool create_pipelines();

	bool rebuild_swapchain();

	VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat find_depth_format();

	VkShaderModule create_shader(const std::vector<uint32_t> &code);

	// this is fucking terrible, what do else do i do. kys retard.
	void image_barrier(
						VkCommandBuffer command, VkImage image,
						VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
						VkImageLayout old_image_layout, VkImageLayout new_image_layout,
						VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
						VkImageSubresourceRange resource_range
	);

	bool build_vertex_layout();
	bool build_vertex_pipelines();
private:
	std::deque<std::function<void()>> deletion_queue = {};
	GameGlobals *game = nullptr;

	vkb::Instance instance = {};
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	vkb::Device device = {};

	VkQueue graphics_queue = VK_NULL_HANDLE;
	VkQueue present_queue = VK_NULL_HANDLE;
	uint32_t graphics_queue_index = UINT32_MAX;

	vkb::Swapchain swapchain = {};
	std::vector<VkImage> swapchain_images = {};
	std::vector<VkImageView> swapchain_image_views = {};

	VkCommandPool command_pool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> command_buffers = {};

	std::vector<VkSemaphore> available_semaphores = {};
	std::vector<VkSemaphore> finished_semaphores = {};
	std::vector<VkFence> in_flight_fences = {};

	const uint32_t MAX_FRAMES_IN_FLIGHT = 3;
	uint32_t image_index = 0;
	uint32_t current_frame = 0;

	VkDescriptorSetLayout descriptor_layout = VK_NULL_HANDLE;

	VmaAllocator allocator = VK_NULL_HANDLE;

	EImage depth_image = {};
	VkImageView depth_image_view = {};

	const uint32_t MAX_OBJECTS = 1024*1024;

	VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;

	std::vector<EBuffer> camera_data_buffers = {};
	std::vector<EBuffer> object_data_buffers = {};

	VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
	std::map<std::string, VkPipelineLayout> pipeline_layouts = {};
	std::map<std::string, VkPipeline> pipelines = {};
private:
	void get_vulkan_extensions();

    template<typename T> T get_instance_proc(const char *proc) {
        assert(instance.instance != VK_NULL_HANDLE); // instance.instance == VkInstance, not vkb::Instance. Major difference.
        T ret = reinterpret_cast<T>(vkGetInstanceProcAddr(instance, proc));
        if(ret != nullptr)
            return ret;

        spdlog::error("Couldn't get instance procedure: {}", proc);

        return nullptr;
    }

    template<typename T> T get_device_proc(const char *proc) {
        assert(device.device != VK_NULL_HANDLE); // same as above, so as below.
        T ret = reinterpret_cast<T>(vkGetDeviceProcAddr(device, proc));
        if(ret != nullptr)
            return ret;

        spdlog::error("Couldn't get device procedure: {}", proc);

        return nullptr;
    }

    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR = nullptr;
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR = nullptr;
    PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR = nullptr;
    PFN_vkGetDescriptorSetLayoutSizeEXT vkGetDescriptorSetLayoutSizeEXT = nullptr;
    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR = nullptr;
    PFN_vkGetDescriptorEXT vkGetDescriptorEXT = nullptr;
    PFN_vkCmdBindDescriptorBuffersEXT vkCmdBindDescriptorBuffersEXT = nullptr;
    PFN_vkCmdSetDescriptorBufferOffsetsEXT vkCmdSetDescriptorBufferOffsetsEXT = nullptr;

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = {};
};

}