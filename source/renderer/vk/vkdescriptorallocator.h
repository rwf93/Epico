#pragma once

class VulkanDevice;
class VulkanDescriptorAllocator {
public:
    void init(VulkanDevice *vkdevice);
private:
    VulkanDevice *device;
};