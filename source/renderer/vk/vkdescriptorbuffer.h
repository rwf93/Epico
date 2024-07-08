#pragma once

class VulkanDevice;
class VulkanBuffer;
class VulkanDescriptorAllocator;
class VulkanUniformBuffer: public AbstractUniform {
public:
    VulkanUniformBuffer();

    void init();
    void fini() override;
    UniformState get_state() override { return state; };
private:
    VulkanDevice *device;

    VulkanDescriptorAllocator allocator;

    UniformState state = UniformState::UNIFORM_READY;
};