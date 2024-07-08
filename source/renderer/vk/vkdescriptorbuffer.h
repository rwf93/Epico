#pragma once

#include "vkdescriptorallocator.h"

class VulkanDevice;
class VulkanBuffer;
class VulkanUniformBuffer: public AbstractUniform {
public:
    ~VulkanUniformBuffer() override;

    void init(VulkanDevice *vkdevice, VulkanBuffer *vkbuffer);
    void fini() override;
    UniformState get_state() override { return state; };

    void clear();

    VK_TRACY_MEMORY_OVERLOADS;

    friend class VulkanDescriptorManager;
private:
    VulkanDevice *device;
    VulkanBuffer *buffer;

    ResourceHandle resource_handle;
    VulkanDescriptorAllocator allocator;

    UniformState state = UniformState::UNIFORM_UNREADY;
};