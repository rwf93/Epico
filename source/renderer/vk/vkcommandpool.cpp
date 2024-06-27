
#include "vkinfo.h"
#include "vkcommandpool.h"
#include "vkdevice.h"
#include "vkswapchain.h"

VulkanCommandPool::VulkanCommandPool() {}
VulkanCommandPool::~VulkanCommandPool() {}

void VulkanCommandPool::init(FunctorQueue<> &queue, VulkanDevice *vkdevice, VulkanSwapchain *vkswapchain) {
	this->device = vkdevice;
	this->swapchain = vkswapchain;

	max_flying_frames = static_cast<uint32_t>(swapchain->get_swapchain_images().size());
	frame_contexts.resize(max_flying_frames);

	create_command_pool();
	create_sync_objects();

	queue.push([&] { fini(); });
}

void VulkanCommandPool::fini() {
	for(uint32_t i = 0; i < get_max_flying_frames(); i++) {
		VulkanFrameContext &context = get_frame_context(i);

		vkDestroyFence(device->get_device(), context.fence, nullptr);
		vkDestroySemaphore(device->get_device(), context.finished_semaphore,  nullptr);
		vkDestroySemaphore(device->get_device(), context.available_semaphore, nullptr);
		vkDestroyCommandPool(device->get_device(), context.command_pool, nullptr);
	}

	vkDestroyFence(device->get_device(), immediate_fence, nullptr);
	vkDestroyCommandPool(device->get_device(), immediate_command_pool, nullptr);
}

void VulkanCommandPool::rebuild() {
	for(uint32_t i = 0; i < get_max_flying_frames(); i++)
		vkDestroyCommandPool(device->get_device(), get_frame_context(i).command_pool, nullptr);

	vkDestroyCommandPool(device->get_device(), immediate_command_pool, nullptr);

	create_command_pool();
}

void VulkanCommandPool::wait_fences() {
	VK_CHECK(vkWaitForFences(device->get_device(), 1, &get_fence(), VK_TRUE, UINT64_MAX));
}

void VulkanCommandPool::reset_fences() {
	VK_CHECK(vkResetFences(device->get_device(), 1, &get_fence()));
}

void VulkanCommandPool::begin_recording() {
	VK_CHECK(vkResetCommandBuffer(get_command(), 0));

	static VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(get_command(), &begin_info));
}

void VulkanCommandPool::end_recording() {
	VK_CHECK(vkEndCommandBuffer(get_command()));
}

void VulkanCommandPool::submit_command(SubmitCommandFunction &&command_function) {
	VK_CHECK(vkResetFences(device->get_device(), 1, &immediate_fence));
	VK_CHECK(vkResetCommandBuffer(immediate_command_buffer, 0));

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(immediate_command_buffer, &begin_info));

	command_function(immediate_command_buffer);

	VK_CHECK(vkEndCommandBuffer(immediate_command_buffer));

	auto command_info = info::command_buffer_submit_info(immediate_command_buffer);
	auto submit_info = info::submit_info(&command_info, nullptr, nullptr);

	VK_CHECK(vkQueueSubmit2(device->get_graphics_queue(), 1, &submit_info, immediate_fence));
	VK_CHECK(vkWaitForFences(device->get_device(), 1, &immediate_fence, true, UINT32_MAX));
}

void VulkanCommandPool::transition_image(
	VkCommandBuffer command,
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

	VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
		? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	image_barrier.subresourceRange = info::image_subresource_range(aspect_mask);
	image_barrier.image = image;

	VkDependencyInfo dependency_info = {};
	dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependency_info.imageMemoryBarrierCount = 1;
	dependency_info.pImageMemoryBarriers = &image_barrier;

	vkCmdPipelineBarrier2(command, &dependency_info);
}

void VulkanCommandPool::copy_image(VkCommandBuffer command, VkImage src, VkImage dst, VkExtent2D src_size, VkExtent2D dst_size) {
	VkImageBlit2 blit = {};
	blit.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;

	blit.srcOffsets[1].x = src_size.width;
	blit.srcOffsets[1].y = src_size.height;
	blit.srcOffsets[1].z = 1;
	blit.dstOffsets[1].x = dst_size.width;
	blit.dstOffsets[1].y = dst_size.height;
	blit.dstOffsets[1].z = 1;

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

	vkCmdBlitImage2(command, &blit_info);
}

void VulkanCommandPool::create_command_pool() {
	auto command_pool_info = info::command_pool_create_info(
		device->get_graphics_queue_index(),
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
	);

	for(uint32_t i = 0; i < get_max_flying_frames(); i++) {
		VulkanFrameContext &context = get_frame_context(i);

		VK_CHECK(vkCreateCommandPool(device->get_device(), &command_pool_info, nullptr, &context.command_pool));
		auto command_allocate_info = info::command_buffer_allocate_info(context.command_pool);
		VK_CHECK(vkAllocateCommandBuffers(device->get_device(),  &command_allocate_info, &context.command_buffer));
	}

	VK_CHECK(vkCreateCommandPool(device->get_device(), &command_pool_info, nullptr, &immediate_command_pool));
	auto immediate_command_buffer_info = info::command_buffer_allocate_info(immediate_command_pool);
	VK_CHECK(vkAllocateCommandBuffers(device->get_device(), &immediate_command_buffer_info, &immediate_command_buffer));
}

void VulkanCommandPool::create_sync_objects() {
	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for(uint32_t i = 0; i < get_max_flying_frames(); i++) {
		VulkanFrameContext &context = get_frame_context(i);

		VK_CHECK(vkCreateFence(device->get_device(), &fence_info, nullptr, &context.fence));
		VK_CHECK(vkCreateSemaphore(device->get_device(), &semaphore_info, nullptr, &context.available_semaphore));
		VK_CHECK(vkCreateSemaphore(device->get_device(), &semaphore_info, nullptr, &context.finished_semaphore));
	}

	VK_CHECK(vkCreateFence(device->get_device(), &fence_info, nullptr, &immediate_fence));
}