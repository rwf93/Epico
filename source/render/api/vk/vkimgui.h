#pragma once

class VulkanInstance;
class VulkanDevice;
class VulkanSwapchain;
class VulkanCommandPool;
class VulkanUI: public RenderUI {
public:
	VulkanUI();
	~VulkanUI() override;

	void process_event(SDL_Event *event);

	void begin() override;
	void end() override;

	void *get_context() override;

	void init(
		FunctorQueue<> &queue,
		AppContext *app_context,
		VulkanInstance *vkinstance ,
		VulkanDevice *vkdevice,
		VulkanSwapchain *vkswapchain,
		VulkanCommandPool *vkcommandpool
	);
	void fini();

private:
	AppContext *context = nullptr;
	VulkanInstance *instance = nullptr;
	VulkanDevice *device = nullptr;
	VulkanSwapchain *swapchain = nullptr;
	VulkanCommandPool *command_pool = nullptr;

	VkDescriptorPool descriptor_pool;
};