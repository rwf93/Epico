#include "tools.h"
#include "info.h"

#include "vkcommandpool.h"
#include "vkdevice.h"
#include "vkswapchain.h"

VulkanCommandPool::VulkanCommandPool(VulkanDevice *device, VulkanSwapchain *swapchain) {
    this->device = device;
    this->swapchain = swapchain;

    max_flying_frames = static_cast<uint32_t>(swapchain->get_swapchain_images().size());
    command_buffers.resize(max_flying_frames);

    create_command_pool();
}

VulkanCommandPool::~VulkanCommandPool() {
    vkDestroyCommandPool(device->get_device(), command_pool, nullptr);
}

void VulkanCommandPool::rebuild() {
    vkDestroyCommandPool(device->get_device(), command_pool, nullptr);
    create_command_pool();
}

void VulkanCommandPool::begin_recording() {
    vkResetCommandBuffer(get_command(), 0);

    static VkCommandBufferBeginInfo begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    VK_CHECK(vkBeginCommandBuffer(get_command(), &begin_info));
}

void VulkanCommandPool::end_recording() {
    VK_CHECK(vkEndCommandBuffer(get_command()));
    current_frame = (current_frame + 1) % max_flying_frames;
}

void VulkanCommandPool::submit_command(SubmitCommandFunction &&command_function) {
    VkCommandBuffer command = {};

    auto command_buffer_info = info::command_buffer_allocate_info(command_pool);
    VK_CHECK(vkAllocateCommandBuffers(device->get_device(), &command_buffer_info, &command));

    VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(command, &begin_info));

    command_function(command);

    VK_CHECK(vkEndCommandBuffer(command));

    VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command;

    VK_CHECK(vkQueueSubmit(device->get_graphics_queue(), 1, &submit_info, VK_NULL_HANDLE));
	VK_CHECK(vkQueueWaitIdle(device->get_graphics_queue()));

    vkFreeCommandBuffers(device->get_device(), command_pool, 1, &command);
}

void VulkanCommandPool::create_command_pool() {
    auto command_pool_info = info::command_pool_create_info(device->get_graphics_queue_index(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VK_CHECK(vkCreateCommandPool(device->get_device(), &command_pool_info, nullptr, &command_pool));

    auto command_allocate_info = info::command_buffer_allocate_info(command_pool, static_cast<uint32_t>(command_buffers.size()));
    VK_CHECK(vkAllocateCommandBuffers(device->get_device(), &command_allocate_info, command_buffers.data()));
}