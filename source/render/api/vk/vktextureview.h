#pragma once

class VulkanDevice;
class VulkanTexture;
class VulkanTextureView: public RenderResource {
public:
    VulkanTextureView() {};
    ~VulkanTextureView() override {};

    ResourceState get_state() override { return state; }

    void init(VulkanDevice *vkdevice, VulkanTexture *vktexture, VkImageViewCreateInfo *view_info);
    void fini();

    VkImageView get_view() { return view; }

private:
    VulkanDevice *device;
    VulkanTexture *texture; // The view is "bound" to this texture

    VkImageView view;
    ResourceState state = ResourceState::UNREADY;
};