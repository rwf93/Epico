#include "tools.h"
#include "info.h"

#include "vkcommandpool.h"
#include "vkdevice.h"
#include "vkswapchain.h"

VulkanCommandPool::VulkanCommandPool(VulkanDevice *device, VulkanSwapchain *swapchain) {
    this->device = device;
    this->swapchain = swapchain;

    max_flying_frames = swapchain->get_swapchain_images().size();
    command_buffers.resize(max_flying_frames);

    auto command_pool_info = info::command_pool_create_info(device->get_graphics_queue_index(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VK_CHECK(vkCreateCommandPool(device->get_device(), &command_pool_info, nullptr, &command_pool));

    auto command_allocate_info = info::command_buffer_allocate_info(command_pool, static_cast<uint32_t>(command_buffers.size()));
    VK_CHECK(vkAllocateCommandBuffers(device->get_device(), &command_allocate_info, command_buffers.data()));
}

VulkanCommandPool::~VulkanCommandPool() {
    vkDestroyCommandPool(device->get_device(), command_pool, nullptr);
}

void VulkanCommandPool::rebuild() {
    vkDestroyCommandPool(device->get_device(), command_pool, nullptr);

    auto command_pool_info = info::command_pool_create_info(device->get_graphics_queue_index(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VK_CHECK(vkCreateCommandPool(device->get_device(), &command_pool_info, nullptr, &command_pool));

    auto command_allocate_info = info::command_buffer_allocate_info(command_pool, static_cast<uint32_t>(command_buffers.size()));
    VK_CHECK(vkAllocateCommandBuffers(device->get_device(), &command_allocate_info, command_buffers.data()));
}

void VulkanCommandPool::begin_recording() {

}

void VulkanCommandPool::end_recording() {

}

void VulkanCommandPool::submit_command(SubmitCommandFunction &&command_function) {

}