#pragma once

class VulkanDevice;
class VulkanTexture;
class VulkanTextureView: public RenderResource {
public:
	VulkanTextureView(VulkanDevice *vkdevice)
		: device(vkdevice) {}
	~VulkanTextureView() override { if(get_state() != ResourceState::UNREADY) fini(); }

	VK_TRACY_MEMORY_OVERLOADS;

	ResourceState get_state() override { return state; }

	void init(VkImageViewCreateInfo *view_info);
	void fini();

	VkImageView get_view() { return view; }

private:
	VulkanDevice *device;

	VkImageView view;
	ResourceState state = ResourceState::UNREADY;
};