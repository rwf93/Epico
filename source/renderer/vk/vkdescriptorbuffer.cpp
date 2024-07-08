#include "vkdevice.h"
#include "vkbuffer.h"
#include "vkdescriptorbuffer.h"

VulkanUniformBuffer::~VulkanUniformBuffer() {

}

void VulkanUniformBuffer::init(VulkanDevice *vkdevice, VulkanBuffer *vkbuffer) {
    this->device = vkdevice;
    this->buffer = vkbuffer;

    std::vector<VulkanDescriptorAllocator::PoolSizeRatio> ratios = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
    };
    allocator.init(device, 1000, ratios);
    state = UniformState::UNIFORM_READY;
}

void VulkanUniformBuffer::clear() {
    allocator.clear();
}

void VulkanUniformBuffer::fini() {
    allocator.fini();
    state = UniformState::UNIFORM_UNREADY;
}