#pragma once

class VulkanDevice;
class VulkanLayout: public RenderResource {
public:
    void init(VulkanDevice *vkdevice, VkDescriptorSetLayoutCreateInfo *layout_info);
    void fini() override;

    ResourceState get_state() { return state; }

    VkDescriptorSetLayout get_layout() { return layout; }
    VkPipelineLayout get_pipeline_layout() { return pipeline_layout; }

private:
    VulkanDevice *device;
    VkDescriptorSetLayout layout;
    VkPipelineLayout pipeline_layout;

    ResourceState state = ResourceState::UNREADY;
};