#pragma once

class VulkanDevice;
class VulkanLayout: public RenderLayout {
public:
    void init(VulkanDevice *vkdevice, VkDescriptorSetLayoutCreateInfo *layout_info);
    void fini() override;

    LayoutState get_state() { return state; }

    VkDescriptorSetLayout get_layout() { return layout; }

private:
    VulkanDevice *device;
    VkDescriptorSetLayout layout;

    LayoutState state = LayoutState::LAYOUT_UNREADY;
};