#include "vkdevice.h"
#include "vktexture.h"
#include "vktextureview.h"

void VulkanTextureView::init(VulkanDevice *vkdevice, VulkanTexture *vktexture, VkImageViewCreateInfo *view_info) {
    this->device = vkdevice;
    this->texture = vktexture;

    VK_CHECK(vkCreateImageView(device->get_device(), view_info, nullptr, &view));

    state = ResourceState::READY;
}

void VulkanTextureView::fini() {
    vkDestroyImageView(device->get_device(), view, nullptr);
    state = ResourceState::UNREADY;
}