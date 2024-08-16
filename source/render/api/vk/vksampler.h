#pragma once

class VulkanDevice;
class VulkanSampler: public RenderResource {
public:
    ~VulkanSampler() override { if(get_state() != ResourceState::UNREADY) fini(); }

    void init(VulkanDevice *vkdevice, VkSamplerCreateInfo *create_info);
	void fini();

    VkSampler &get_sampler() { return sampler; };
	ResourceState get_state() { return state; };

private:
    VulkanDevice *device;

    VkSampler sampler;
    ResourceState state = ResourceState::UNREADY;
};