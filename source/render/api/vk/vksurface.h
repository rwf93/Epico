#pragma once

class VulkanInstance;
class VulkanSurface {
public:
	VulkanSurface(AppContext *app_context, VulkanInstance *vkinstance);
	~VulkanSurface();

	VkSurfaceKHR &get_surface() { return surface; }
private:
	VulkanInstance *instance = nullptr;
	AppContext *context = nullptr;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
};