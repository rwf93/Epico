#pragma once

#include "vkdescriptorallocator.h"

class VulkanDevice;
class VulkanBuffer;
class VulkanUniformBuffer: public AbstractUniform {
public:
    ~VulkanUniformBuffer() override;

    void init();
    void fini() override;
    UniformState get_state() override { return state; };

    friend class VulkanDescriptorManager;
private:
    VulkanDevice *device;

    ResourceHandle resource_handle;
    VulkanDescriptorAllocator allocator;

    UniformState state = UniformState::UNIFORM_READY;
};