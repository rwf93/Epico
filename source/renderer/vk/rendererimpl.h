#pragma once

class VulkanRenderer: public AbstractRenderer {
public:
	VulkanRenderer(AppContext *app_context);
	~VulkanRenderer() override;

	void begin() override;
	void end() override;
	void present() override;

	void begin_pass(SubpassDependency *dependencies) override;
	void end_pass(SubpassDependency *dependencies) override;

	void clear(float r, float g, float b, float a) override;
	void viewport(float width, float height, float x = 0, float y = 0) override;
	void scissor(uint32_t width, uint32_t height, int32_t x = 0, int32_t y = 0) override;

	void bind_buffer(ResourceHandle handle, BindBufferType type) override;
	void bind_graphic_shader(ShaderHandle handle) override;

	void draw(uint32_t vertex_count, uint32_t instance_count) override;
	void draw_instanced(uint32_t index_count, uint32_t instance_count, uint32_t index) override;

	void show_image(ResourceHandle handle) override;

	void on_resize(ResizeEventFunction &&resize) override {
		resize_event = resize;
	};

	ResourceHandle create_image() override;
	ResourceHandle create_buffer() override;

	UniformHandle create_uniform_buffer() override;
	void init_uniform_buffer(UniformHandle uniform_handle, ResourceHandle resource_handle) override;

	AbstractGraphicShaderBuilder *create_graphic_shader() override;

	void buffer_data(ResourceHandle handle, BufferType type, size_t size, void *data) override;
	void buffer_sub_data(ResourceHandle handle, size_t offset, size_t size, void *data) override;

	void image_data(
		ResourceHandle handle,
		ImageDimensions dimensions,
		ImageSamples samples,
		ImageFormat format,
		ImageFlags flags,
		void *data,
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
	VulkanDescriptorManager descriptor_manager = {};
	VulkanShaderManager shader_manager = {};

	VulkanImGUI ui_imgui = {};

	ResizeEventFunction resize_event;
};