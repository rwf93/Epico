#pragma once

class VulkanRenderer: public AbstractRenderer {
public:
	VulkanRenderer(AppContext *app_context);
	~VulkanRenderer() override;

	void begin() override;
	void end() override;

	void clear(float r, float g, float b, float a) override;

	AbstractUI *ui() { return &ui_imgui; };

	ResourceHandle create_image(
		int width,
		int height,
		int depth = 1,
		ImageDimensions dimensions = ImageDimensions::IMAGE_2D,
		ImageFormat format = ImageFormat::R16G16B16A16_UINT,
		ImageSample samples = ImageSample::SAMPLE_COUNT_16_BIT
	) override;
protected:
	void rebuild();

private:
	AppContext *app_context = nullptr;

	FunctorQueue<> cleanup_queue;

	VulkanInstance instance = {};
	VulkanSurface surface = {};
	VulkanDevice device = {};
	VulkanSwapchain swapchain = {};
	VulkanCommandPool command_pool = {};
	VulkanImGUI ui_imgui = { &command_pool };

	VulkanResourceManager resource_manager;
};