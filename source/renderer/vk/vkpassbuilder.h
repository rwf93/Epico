#pragma once

class VulkanDevice;
class VulkanPassBuilder: public AbstractPassBuilder {
public:
    VulkanPassBuilder(VulkanDevice *device);
    ~VulkanPassBuilder();

    AbstractPass *build() { return nullptr; }
private:
    VulkanDevice *device;
};