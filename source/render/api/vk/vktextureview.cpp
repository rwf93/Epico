#include "vkdevice.h"
#include "vktexture.h"
#include "vktextureview.h"

void VulkanTextureView::init(VkImageViewCreateInfo *view_info) {
    VK_CHECK(vkCreateImageView(device->get_device(), view_info, nullptr, &view));
    state = ResourceState::READY;
}

void VulkanTextureView::fini() {
    vkDestroyImageView(device->get_device(), view, nullptr);
    state = ResourceState::UNREADY;
}