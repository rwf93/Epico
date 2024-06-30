#pragma once

class VulkanRenderer: public AbstractRenderer {
public:
	VulkanRenderer(AppContext *app_context);
	~VulkanRenderer() override;

	void begin() override;
	void end() override;

	void begin_pass(SubpassDependency *dependencies) override;
	void end_pass() override;

	void clear(ResourceHandle handle, float r, float g, float b, float a) override;
	void present(ResourceHandle handle) override;

	void viewport(float width, float height, float x = 0, float y = 0) override;
	void scissor(uint32_t width, uint32_t height, int32_t x = 0, int32_t y = 0) override;

	ResourceHandle create_image() override;
	ResourceHandle create_buffer() override;

	void buffer_data(ResourceHandle handle, BufferType type, size_t size, void *data) override;
	void buffer_sub_data(ResourceHandle handle, size_t offset, size_t size, void *data) override;

	void image_data(
		ResourceHandle handle,
		ImageDimensions dimensions,
		ImageSamples samples,
		ImageFormat format,
		void *data,
		bool mipmapped,
		int width, int height, int depth
	);

	AbstractUI *ui() { return &ui_imgui; };

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
	VulkanResourceManager resource_manager = {};

	VulkanImGUI ui_imgui = {};
};