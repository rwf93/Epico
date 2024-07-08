#pragma once

class VulkanDevice;
class VulkanCommand {
public:
	void init(VulkanDevice *vkdevice, VkCommandBufferAllocateInfo *allocate_info);
	VkCommandBuffer get_command() { return command; }

	void reset(VkCommandBufferResetFlags reset_flags = 0);

	void begin_recording(VkCommandBufferBeginInfo *begin_info);
	void end_recording();

	void begin_rendering(VkRenderingInfo *rendering_info);
	void end_rendering();

	void transition_image(
		VkImage image,
		VkImageLayout current_layout,
		VkImageLayout new_layout
	);
	void copy_image(
		VkImage src,
		VkImage dst,
		VkExtent3D src_size,
		VkExtent3D dst_size
	);
	void clear_image(
		VkImage image,
		VkImageLayout layout,
		VkClearColorValue *clear_values,
		std::span<VkImageSubresourceRange> ranges
	);

	void copy_buffer_to_image(
		VkBuffer src_buffer,
		VkImage dst_image,
		VkImageLayout layout,
		std::span<VkBufferImageCopy> regions
	);

	void copy_buffer(
		VkBuffer src_buffer,
		VkBuffer dst_buffer,
		std::span<VkBufferCopy> regions
	);

	void update_buffer(
		VkBuffer src_buffer,
		VkDeviceSize offset,
		VkDeviceSize size,
		void *data
	);

	void viewport(
		uint32_t first_viewport,
		std::span<VkViewport> viewports
	);
	void scissor(
		uint32_t first_scissor,
		std::span<VkRect2D> scissors
	);

	void bind_vertex_buffer(
		uint32_t first_binding,
		uint32_t binding_count,
		VkBuffer *buffers,
		VkDeviceSize *offsets
	);
	void bind_index_buffer(
		VkBuffer buffer,
		VkDeviceSize offset,
		VkIndexType type
	);
	void bind_pipeline(VkPipelineBindPoint point, VkPipeline pipeline);

	void draw(
		uint32_t vertex_count,
		uint32_t instance_count,
		uint32_t first_vertex,
		uint32_t first_instance
	);
	void draw_instanced(
		uint32_t index_count,
		uint32_t instance_count,
		uint32_t first_index,
		uint32_t vertex_offset,
		uint32_t first_instance
	);

	friend class VulkanCommandPool;

private:
	VkCommandBuffer command;
};