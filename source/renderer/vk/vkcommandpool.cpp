
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
    vkDeviceWaitIdle(device->get_device());

    for(uint32_t i = 0; i < get_max_flying_frames(); i++) {
        VulkanFrameContext &context = get_frame_context(i);

		vkDestroyFence(device->get_device(), context.fence,               nullptr);
		vkDestroySemaphore(device->get_device(), context.finished_semaphore,  nullptr);
        vkDestroySemaphore(device->get_device(), context.available_semaphore, nullptr);
        vkDestroyCommandPool(device->get_device(), context.command_pool,        nullptr);
    }

    vkDestroyFence(device->get_device(), immediate_fence, nullptr);
    vkDestroyCommandPool(device->get_device(), immediate_command_pool, nullptr);
}

void VulkanCommandPool::rebuild() {
    vkDeviceWaitIdle(device->get_device());

    for(uint32_t i = 0; i < get_max_flying_frames(); i++)
        vkDestroyCommandPool(device->get_device(), get_frame_context(i).command_pool, nullptr);

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

    command_function(this, immediate_command_buffer);

    VK_CHECK(vkEndCommandBuffer(immediate_command_buffer));

    auto command_info = info::command_buffer_submit_info(immediate_command_buffer);
    auto submit_info = info::submit_info(&command_info, nullptr, nullptr);

    VK_CHECK(vkQueueSubmit2(device->get_graphics_queue(), 1, &submit_info, immediate_fence));
    VK_CHECK(vkWaitForFences(device->get_device(), 1, &immediate_fence, true, UINT32_MAX));
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
        VK_CHECK(vkAllocateCommandBuffers(device->get_device(), &command_allocate_info, &context.command_buffer));
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