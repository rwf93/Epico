#include "vkdevice.h"
#include "vkcommand.h"

void VulkanCommand::init(VulkanDevice *vkdevice, VkCommandBufferAllocateInfo *allocate_info) {
	VK_CHECK(vkAllocateCommandBuffers(vkdevice->get_device(),  allocate_info, &command));
}

void VulkanCommand::reset(VkCommandBufferResetFlags reset_flags) {
	VK_CHECK(vkResetCommandBuffer(get_command(), reset_flags));
}

void VulkanCommand::begin_recording(VkCommandBufferBeginInfo *begin_info) {
	VK_CHECK(vkBeginCommandBuffer(get_command(), begin_info));
}

void VulkanCommand::end_recording() {
	VK_CHECK(vkEndCommandBuffer(get_command()));
}

void VulkanCommand::begin_rendering(VkRenderingInfo *rendering_info) {
	vkCmdBeginRendering(get_command(), rendering_info);
}

void VulkanCommand::end_rendering() {
	vkCmdEndRendering(get_command());
}

void VulkanCommand::transition_image(
	VkImage image,
	VkImageLayout current_layout,
	VkImageLayout new_layout
) {
	VkImageMemoryBarrier2 image_barrier = {};
	image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	image_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	image_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
	image_barrier.oldLayout = current_layout;
	image_barrier.newLayout = new_layout;

	VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL || current_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
		? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	image_barrier.subresourceRange = info::image_subresource_range(aspect_mask);
	image_barrier.image = image;

	VkDependencyInfo dependency_info = {};
	dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependency_info.imageMemoryBarrierCount = 1;
	dependency_info.pImageMemoryBarriers = &image_barrier;

	vkCmdPipelineBarrier2(get_command(), &dependency_info);
}

void VulkanCommand::copy_image(
	VkImage src,
	VkImage dst,
	VkExtent3D src_size,
	VkExtent3D dst_size
) {
	VkImageBlit2 blit = {};
	blit.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;

	blit.srcOffsets[1].x = src_size.width;
	blit.srcOffsets[1].y = src_size.height;
	blit.srcOffsets[1].z = src_size.depth;

	blit.dstOffsets[1].x = dst_size.width;
	blit.dstOffsets[1].y = dst_size.height;
	blit.dstOffsets[1].z = dst_size.depth;

	blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.srcSubresource.baseArrayLayer = 0;
	blit.srcSubresource.layerCount = 1;
	blit.srcSubresource.mipLevel = 0;

	blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.dstSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount = 1;
	blit.dstSubresource.mipLevel = 0;

	VkBlitImageInfo2 blit_info = {};
	blit_info.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
	blit_info.srcImage = src;
	blit_info.dstImage = dst;
	blit_info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blit_info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blit_info.filter = VK_FILTER_LINEAR;
	blit_info.regionCount = 1;
	blit_info.pRegions = &blit;

	vkCmdBlitImage2(get_command(), &blit_info);
}

void VulkanCommand::clear_image(
	VkImage image,
	VkImageLayout layout,
	VkClearColorValue *clear_values,
	std::span<VkImageSubresourceRange> ranges
) {
	vkCmdClearColorImage(
		get_command(),
		image,
		layout,
		clear_values,
		static_cast<uint32_t>(ranges.size()), ranges.data()
	);
}

void VulkanCommand::copy_buffer_to_image(
	VkBuffer src_buffer,
	VkImage dst_image,
	VkImageLayout layout,
	std::span<VkBufferImageCopy> regions
) {
	vkCmdCopyBufferToImage(
		get_command(),
		src_buffer,
		dst_image,
		layout,
		static_cast<uint32_t>(regions.size()),
		regions.data()
	);
}

void VulkanCommand::copy_buffer(
	VkBuffer src_buffer,
	VkBuffer dst_buffer,
	std::span<VkBufferCopy> regions
) {
	vkCmdCopyBuffer(
		get_command(),
		src_buffer,
		dst_buffer,
		static_cast<uint32_t>(regions.size()),
		regions.data()
	);
}

void VulkanCommand::update_buffer(
	VkBuffer src_buffer,
	VkDeviceSize offset,
	VkDeviceSize size,
	void *data
) {
	vkCmdUpdateBuffer(get_command(), src_buffer, offset, size, data);
}

void VulkanCommand::viewport(
	uint32_t first_viewport,
	std::span<VkViewport> viewports
) {
	vkCmdSetViewport(
		get_command(),
		first_viewport,
		static_cast<uint32_t>(viewports.size()),
		viewports.data()
	);
}

void VulkanCommand::scissor(
	uint32_t first_scissor,
	std::span<VkRect2D> scissors
) {
	vkCmdSetScissor(
		get_command(),
		first_scissor,
		static_cast<uint32_t>(scissors.size()),
		scissors.data()
	);
}

void VulkanCommand::bind_vertex_buffer(
	uint32_t first_binding,
	uint32_t binding_count,
	VkBuffer *buffers,
	VkDeviceSize *offsets
) {
	vkCmdBindVertexBuffers(get_command(), first_binding, binding_count, buffers, offsets);
}

void VulkanCommand::bind_index_buffer(
	VkBuffer buffer,
	VkDeviceSize offset,
	VkIndexType type
) {
	vkCmdBindIndexBuffer(
		get_command(),
		buffer,
		offset,
		type
	);
}

void VulkanCommand::bind_pipeline(VkPipelineBindPoint point, VkPipeline pipeline) {
	vkCmdBindPipeline(get_command(), point, pipeline);
}

void VulkanCommand::draw(
	uint32_t vertex_count,
	uint32_t instance_count,
	uint32_t first_vertex,
	uint32_t first_instance
) {
	vkCmdDraw(
		get_command(),
		vertex_count,
		instance_count,
		first_vertex,
		first_instance
	);
}

void VulkanCommand::draw_instanced(
	uint32_t index_count,
	uint32_t instance_count,
	uint32_t first_index,
	uint32_t vertex_offset,
	uint32_t first_instance
) {
	vkCmdDrawIndexed(
		get_command(),
		index_count,
		instance_count,
		first_index,
		vertex_offset,
		first_instance
	);
}