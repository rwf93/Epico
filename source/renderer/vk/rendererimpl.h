#pragma once

class VulkanRenderer: public AbstractRenderer {
public:
    VulkanRenderer(AppContext *app_context);
    ~VulkanRenderer() override;

    void begin() override;
    void end() override;

    void clear(float r, float g, float b, float a) override;

    AbstractUI *ui() { return &ui_imgui; };

protected:
    void rebuild();

private:
    AppContext *app_context = nullptr;

    VulkanInstance instance = {};
    VulkanSurface surface = { &instance };
    VulkanDevice device = { &instance, &surface };
    VulkanSwapchain swapchain = { &device };
    VulkanCommandPool command_pool = { &device, &swapchain };
    VulkanImGUI ui_imgui = { &command_pool };
};