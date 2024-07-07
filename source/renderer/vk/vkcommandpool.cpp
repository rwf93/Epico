
#include "vkinfo.h"
#include "vkdevice.h"
#include "vkswapchain.h"
#include "vkcommandpool.h"

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
	static VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	get_command()->reset();
	get_command()->begin_recording(&begin_info);
}

void VulkanCommandPool::end_recording() {
	get_command()->end_recording();
}

void VulkanCommandPool::submit_command(SubmitCommandFunction &&command_function) {
	VK_CHECK(vkResetFences(device->get_device(), 1, &immediate_fence));
	immediate_command_buffer.reset();

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	immediate_command_buffer.begin_recording(&begin_info);

	command_function(&immediate_command_buffer);

	immediate_command_buffer.end_recording();

	auto command_info = info::command_buffer_submit_info(immediate_command_buffer.get_command());
	auto submit_info = info::submit_info(&command_info, nullptr, nullptr);

	VK_CHECK(vkQueueSubmit2(device->get_graphics_queue(), 1, &submit_info, immediate_fence));
	VK_CHECK(vkWaitForFences(device->get_device(), 1, &immediate_fence, true, UINT32_MAX));
}

void VulkanCommandPool::transition_image(
	VulkanCommand *command,
	VkImage image,
	VkImageLayout current_layout,
	VkImageLayout new_layout
) {
	command->transition_image(image, current_layout, new_layout);
}

void VulkanCommandPool::copy_image(VulkanCommand *command, VkImage src, VkImage dst, VkExtent3D src_size, VkExtent3D dst_size) {
	command->copy_image(src, dst, src_size, dst_size);
}

void VulkanCommandPool::clear_image(VulkanCommand *command, VkImage image, float r, float g, float b, float a) {
	VkClearColorValue clear_value = { { r, g, b, a } };
	std::vector<VkImageSubresourceRange> clear_ranges = {
		info::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT)
	};

	command->clear_image(image, VK_IMAGE_LAYOUT_GENERAL, &clear_value, clear_ranges);
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
		context.command_buffer.init(device, &command_allocate_info);
	}

	VK_CHECK(vkCreateCommandPool(device->get_device(), &command_pool_info, nullptr, &immediate_command_pool));
	auto immediate_command_buffer_info = info::command_buffer_allocate_info(immediate_command_pool);
	immediate_command_buffer.init(device, &immediate_command_buffer_info);
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