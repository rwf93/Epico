#pragma once

class VulkanDevice;
class VulkanTexture;
class VulkanTextureView: public RenderResource {
public:
    VulkanTextureView() = default;
    ~VulkanTextureView() override { if(get_state() != ResourceState::UNREADY) fini(); }

    ResourceState get_state() override { return state; }

    void init(VulkanDevice *vkdevice, VkImageViewCreateInfo *view_info);
    void fini();

    VkImageView get_view() { return view; }

private:
    VulkanDevice *device;

    VkImageView view;
    ResourceState state = ResourceState::UNREADY;
};