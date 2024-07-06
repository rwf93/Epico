#include "vkdescriptorallocator.h"

void VulkanDescriptorAllocator::init(VulkanDevice *vkdevice) {
    this->device = vkdevice;
}
