#include "vkdevice.h"
#include "vkcommandpool.h"
#include "vkbuffer.h"

VulkanBuffer::VulkanBuffer(
    VulkanDevice *vkdevice,
    VulkanCommandPool *vkcommandpool,
    VmaAllocator vkallocator
) {
    this->device = vkdevice;
    this->command_pool = vkcommandpool;
    this->allocator = vkallocator;
}

VulkanBuffer::~VulkanBuffer() {}

void VulkanBuffer::init(
    VkBufferCreateInfo *buffer_create_info,
    VmaAllocationCreateInfo *allocation_create_info
) {
    VK_CHECK(vmaCreateBuffer(
        allocator,
        buffer_create_info, allocation_create_info,
        &buffer, &allocation, &allocation_info
    ));

    prepared = true;
}

void VulkanBuffer::stage(
    VulkanBuffer *staging_buffer,
    VkDeviceSize size,
    VkDeviceSize src_offset,
    VkDeviceSize dst_offset
) {
    assert(is_prepared());

    command_pool->submit_command([&](VkCommandBuffer command) {
        VkBufferCopy copy;
        copy.size = size;
        copy.srcOffset = src_offset;
        copy.dstOffset = dst_offset;

        vmaFlushAllocation(allocator, staging_buffer->get_allocation(), 0, VK_WHOLE_SIZE);
        vkCmdCopyBuffer(command, staging_buffer->get_buffer(), buffer, 1, &copy);
    });
}

void VulkanBuffer::update(void *data, VkDeviceSize size, VkDeviceSize offset) {
    assert(is_prepared());

    command_pool->submit_command([&](VkCommandBuffer command) {
        vkCmdUpdateBuffer(command, buffer, offset, size, data);
    });
}

void VulkanBuffer::fini() {
    if(is_prepared())
        vmaDestroyBuffer(allocator, buffer, allocation);
    prepared = false;
}