#include "vkdevice.h"
#include "vklayout.h"

void VulkanLayout::init(VulkanDevice *vkdevice, VkDescriptorSetLayoutCreateInfo *layout_info) {
    this->device = vkdevice;

    VK_CHECK(vkCreateDescriptorSetLayout(device->get_device(), layout_info, nullptr, &layout));

    std::vector<VkDescriptorSetLayout> layouts = { layout };
    auto pipeline_layout_info = info::pipeline_layout_info(layouts);
	VK_CHECK(vkCreatePipelineLayout(device->get_device(), &pipeline_layout_info, nullptr, &pipeline_layout));

    state = LayoutState::LAYOUT_READY;
}

void VulkanLayout::fini() {
    vkDestroyPipelineLayout(device->get_device(), pipeline_layout, nullptr);
    vkDestroyDescriptorSetLayout(device->get_device(), layout, nullptr);

    state = LayoutState::LAYOUT_UNREADY;
}