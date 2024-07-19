#pragma once

class VulkanInstance;
class VulkanDevice;
class VulkanSwapchain;
class VulkanCommandPool;
class VulkanImGUI: public RenderUI {
public:
	VulkanImGUI();
	~VulkanImGUI() override;

	void process_event(SDL_Event *event);

	void begin_ui() override;
	void end_ui() override;

	void begin(const char *name) override;
	void end() override;

	void init(
		FunctorQueue<> &queue,
		AppContext *app_context,
		VulkanInstance *vkinstance ,
		VulkanDevice *vkdevice,
		VulkanSwapchain *vkswapchain,
		VulkanCommandPool *vkcommandpool
	);
	void fini();

	void show_demo_window() override;
private:
	AppContext *context = nullptr;
	VulkanInstance *instance = nullptr;
	VulkanDevice *device = nullptr;
	VulkanSwapchain *swapchain = nullptr;
	VulkanCommandPool *command_pool = nullptr;

	VkDescriptorPool descriptor_pool;
};