#include "vkdevice.h"
#include "vklayout.h"

void VulkanLayout::init(VulkanDevice *vkdevice, VkDescriptorSetLayoutCreateInfo *layout_info) {
    this->device = vkdevice;

    VK_CHECK(vkCreateDescriptorSetLayout(device->get_device(), layout_info, nullptr, &layout));
    state = LayoutState::LAYOUT_READY;
}

void VulkanLayout::fini() {
    vkDestroyDescriptorSetLayout(device->get_device(), layout, nullptr);
    state = LayoutState::LAYOUT_UNREADY;
}