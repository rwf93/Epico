#pragma once

class VulkanAPI: public RenderAPI {
public:
	~VulkanAPI() override;

	void init(AppContext *context) override;

	void begin() override;
	void end() override;
	void present() override;

	void begin_pass(std::span<SubpassAttachment> dependencies) override;
	void end_pass(std::span<SubpassAttachment> dependencies) override;

	void clear(float r, float g, float b, float a) override;
	void clear(TextureHandle handle, float r, float g, float b, float a) override;

	void viewport(float width, float height, float x = 0, float y = 0) override;
	void scissor(uint32_t width, uint32_t height, int32_t x = 0, int32_t y = 0) override;

	void bind_buffer(BufferHandle handle, BindBufferType type) override;
	void bind_shader(GraphicsProgramHandle handle) override;
	void bind_uniform(LayoutHandle layout, std::span<UniformBind> binds) override;

	void draw(uint32_t vertex_count, uint32_t instance_count) override;
	void draw_instanced(uint32_t index_count, uint32_t instance_count, uint32_t index) override;

	void show_image(TextureHandle handle) override;

	void on_resize(ResizeEventFunction &&resize) override {
		resize_event = resize;
	};

	TextureHandle create_texture() override;
	TextureViewHandle create_texture_view() override;
	BufferHandle create_buffer() override;

	void buffer_data(BufferHandle handle, BufferType type, size_t size, void *data) override;
	void buffer_sub_data(BufferHandle handle, size_t offset, size_t size, void *data) override;

	void texture_data(
		TextureHandle handle,
		ImageDimensions dimensions,
		ImageSamples samples,
		ImageFormat format,
		ImageFlags flags,
		void *data,
		int width, int height, int depth
	);

	void texture_view(
		TextureViewHandle view_handle,
		TextureHandle texture_handle,
		ImageViewDimensions dimensions,
		ImageFormat format,
		int min_level,
		int min_layers,
		int num_levels = 1,
		int num_layers = 1
	) override;

	RenderLayoutBuilder *create_layout() override;
	RenderGraphicProgramBuilder *create_graphic_program() override;

	RenderUI *ui() { return &ui_imgui; };

protected:
	void rebuild();
private:
	AppContext *context = nullptr;

	FunctorQueue<> cleanup_queue;

	VulkanInstance instance = {};
	VulkanSurface surface = {};
	VulkanDevice device = {};
	VulkanSwapchain swapchain = {};
	VulkanCommandPool command_pool = {};
	VulkanResourceManager resource_manager = {};
	VulkanLayoutManager layout_manager = {};
	VulkanProgramManager shader_manager = {};

	VulkanImGUI ui_imgui = {};

	ResizeEventFunction resize_event;
};