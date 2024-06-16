#pragma once

class VulkanInstance;
class VulkanSurface {
public:
    VulkanSurface();
    ~VulkanSurface();

    void init(FunctorQueue<> &queue, AppContext *app_context, VulkanInstance *vkinstance);
    void fini();

    VkSurfaceKHR &get_surface() { return surface; }
private:
    VulkanInstance *instance = nullptr;
    AppContext *context = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
};