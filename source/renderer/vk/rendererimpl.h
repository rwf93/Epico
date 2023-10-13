#pragma once

class VulkanRenderer: public AbstractRenderer {
public:
    VulkanRenderer(AppContext *app_context);
    ~VulkanRenderer() override;

    void begin() override;
    void end() override;

    void begin_pass() override;
    void end_pass() override;
private:
    AppContext *app_context = nullptr;

    VulkanInstance instance = {};
    VulkanSurface surface = { &instance };
    VulkanDevice device = { &instance, &surface };
    VulkanSwapchain swapchain = { &device };
    VulkanCommandPool command_pool = { &device, &swapchain };
};