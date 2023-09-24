#pragma once

#include <vk_mem_alloc.h>

#include "pipeline.h"
#include "primitives.h"
#include "voxel.h"

namespace render {

class Renderer {
public:
	Renderer(GameGlobals *game);
	~Renderer();

	bool setup();
	bool draw();

	void submit_command(std::function<void(VkCommandBuffer command)> &&function);
private:
	bool create_instance();
	bool create_surface();
	bool create_device();
	bool create_swapchain();
	bool create_queues();
	bool create_pipeline_cache();
	bool create_descriptor_layout();
	bool create_pipelines();
	bool create_vma_allocator();
	bool create_depth_image();
	bool create_descriptor_pool();
	bool create_uniform_buffers();
	bool create_descriptor_sets();
	bool create_command_pool();
	bool create_sync_objects();

	bool create_imgui();

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
	GameGlobals *game = nullptr;

	vkb::Instance instance = {};
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	vkb::Device device = {};

	vkb::Swapchain swapchain = {};
	std::vector<VkImage> swapchain_images = {};
	std::vector<VkImageView> swapchain_image_views = {};

	VkQueue graphics_queue = VK_NULL_HANDLE;
	VkQueue present_queue = VK_NULL_HANDLE;
	uint32_t graphics_queue_index = UINT32_MAX;

	VkDescriptorSetLayout global_descriptor_layout = VK_NULL_HANDLE;
	VkDescriptorSetLayout object_descriptor_layout = VK_NULL_HANDLE;

	VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
	std::map<std::string, VkPipelineLayout> pipeline_layouts = {};
	std::map<std::string, VkPipeline> pipelines = {};

	VmaAllocator allocator = VK_NULL_HANDLE;

	ETexture depth_texture = {};

	const uint32_t MAX_OBJECTS = 1024*1024;

	struct UniformBufferAllocation {
		EBuffer memory;
		VmaAllocationInfo info;
	};

	VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;

	std::vector<UniformBufferAllocation> camera_data_buffers = {};
	std::vector<UniformBufferAllocation> object_data_buffers = {};

	std::vector<VkDescriptorSet> global_descriptor_sets = {};
	std::vector<VkDescriptorSet> object_descriptor_sets = {};

	VkCommandPool command_pool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> command_buffers = {};

	const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	std::vector<VkSemaphore> available_semaphores = {};
	std::vector<VkSemaphore> finished_semaphores = {};
	std::vector<VkFence> in_flight_fences = {};

	size_t current_frame = 0;

	std::deque<std::function<void()>> deletion_queue = {};

	std::vector<Chunk> chunks = {};

private:
	PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR;
	PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR;
};

}