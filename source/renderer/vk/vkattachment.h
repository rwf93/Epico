#pragma once

class VulkanDevice;
class VulkanAttachment: public AbstractPass {
public:
    VulkanAttachment(VulkanDevice *device);
    ~VulkanAttachment();

    VkRenderPass &get_pass() { return pass; }
private:
    VulkanDevice *device;
    VkRenderPass pass;
    std::vector<VkAttachmentDescription> attachments;
};