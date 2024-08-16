#pragma once

class VulkanInstance;
class VulkanSurface {
public:
	VulkanSurface();
	~VulkanSurface();

	void init(AppContext *app_context, VulkanInstance *vkinstance);

	VkSurfaceKHR &get_surface() { return surface; }
private:
	VulkanInstance *instance = nullptr;
	AppContext *context = nullptr;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
};