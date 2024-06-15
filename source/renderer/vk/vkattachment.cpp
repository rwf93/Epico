#include "vkattachment.h"
#include "vkdevice.h"

VulkanAttachment::VulkanAttachment(VulkanDevice *device) {
    this->device = device;
}

VulkanAttachment::~VulkanAttachment() {
    vkDestroyRenderPass(device->get_device(), pass, nullptr);
}

