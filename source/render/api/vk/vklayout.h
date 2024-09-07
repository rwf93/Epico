#pragma once

class VulkanDevice;
class VulkanLayout: public RenderResource {
public:
    VulkanLayout(VulkanDevice *vkdevice): device(vkdevice) {}
    ~VulkanLayout() override { if(get_state() != ResourceState::UNREADY) fini(); }

    VK_TRACY_MEMORY_OVERLOADS;

    void init(VkDescriptorSetLayoutCreateInfo *layout_info);
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