#pragma once

#include "vkcommand.h"

class VulkanDevice;
class VulkanSwapchain;
class VulkanCommandPool {
public:
	struct VulkanFrameContext {
		VkCommandPool command_pool;
		VulkanCommand command_buffer;

		VkSemaphore available_semaphore;
		VkSemaphore finished_semaphore;
		VkFence fence;

		TracyVkCtx trace_context;
	};

	VulkanCommandPool();
	~VulkanCommandPool();

	void init(FunctorQueue<> &queue, VulkanDevice *vkdevice, VulkanSwapchain *vkswapchain);
	void fini();

	void wait_fences();
	void reset_fences();

	void begin_recording();
	void end_recording();

	VulkanFrameContext &get_frame_context(uint32_t index) { return frame_contexts.at(index); }
	VulkanFrameContext &get_frame_context() { return get_frame_context(current_frame); }

	VulkanCommand *get_command(uint32_t index) { return &get_frame_context(index).command_buffer; }
	VulkanCommand *get_command() { return get_command(current_frame); };

	VkSemaphore &get_available_semaphore(uint32_t index) { return get_frame_context(index).available_semaphore; }
	VkSemaphore &get_available_semaphore() { return get_available_semaphore(current_frame); }

	VkSemaphore &get_finished_semaphore(uint32_t index) { return get_frame_context(index).finished_semaphore; }
	VkSemaphore &get_finished_semaphore() { return get_available_semaphore(current_frame); }

	VkFence &get_fence(uint32_t index) { return get_frame_context(index).fence; }
	VkFence &get_fence() { return get_fence(current_frame); }

	void transition_image(
		VulkanCommand *command,
		VkImage image,
		VkImageLayout current_layout,
		VkImageLayout new_layout
	);

	void transition_image(
		VkImage image,
		VkImageLayout current_layout,
		VkImageLayout new_layout
	) {
		transition_image(get_command(), image, current_layout, new_layout);
	}

	void copy_image(VulkanCommand *command, VkImage src, VkImage dst, VkExtent3D src_size, VkExtent3D dst_size);
	void copy_image(VkImage src, VkImage dst, VkExtent3D src_size, VkExtent3D dst_size) {
		copy_image(get_command(), src, dst, src_size, dst_size);
	}

	void clear_image(VulkanCommand *command, VkImage image, float r, float g, float b, float a);
	void clear_image(VkImage image, float r, float g, float b, float a) {
		clear_image(get_command(), image, r, g, b, a);
	}

	uint32_t get_max_flying_frames() { return max_flying_frames; }

	// Flips the frame for the backbuffer.
	void advance() { current_frame = (current_frame + 1) % get_max_flying_frames(); }

	// Submits a single command to the gpu (useful for doing memory transfers cpu <-> gpu).
	using SubmitCommandFunction = std::function<void(VulkanCommand *command)>;
	void submit_command(SubmitCommandFunction &&command_function);
private:
	void create_command_pool();
	void create_sync_objects();
private:
	VulkanDevice *device = nullptr;
	VulkanSwapchain *swapchain = nullptr;

	std::vector<VulkanFrameContext> frame_contexts;

	VkFence immediate_fence;
	VkCommandPool immediate_command_pool;
	VulkanCommand immediate_command_buffer;
	TracyVkCtx immediate_trace;

	uint32_t max_flying_frames = 0;
	uint32_t current_frame = 0;
};