#pragma once

class VulkanInstance;
class VulkanDevice;
class VulkanSwapchain;
class VulkanCommandPool;
class VulkanResourceManager;
class VulkanUI: public RenderUI {
public:
	VulkanUI(
		AppContext *app_context,
		VulkanInstance *vkinstance ,
		VulkanDevice *vkdevice,
		VulkanSwapchain *vkswapchain,
		VulkanCommandPool *vkcommandpool,
		VulkanResourceManager *vkresourcemanager
	);
	~VulkanUI() override;

	void process_event(SDL_Event *event);

	void begin() override;
	void end() override;

	void *get_context() override;
	void *add_texture(SamplerHandle sampler_handle, TextureViewHandle texture_view_handle) override;

private:
	AppContext *context = nullptr;
	VulkanInstance *instance = nullptr;
	VulkanDevice *device = nullptr;
	VulkanSwapchain *swapchain = nullptr;
	VulkanCommandPool *command_pool = nullptr;
	VulkanResourceManager *resource_manager = nullptr;

	VkDescriptorPool descriptor_pool;
};