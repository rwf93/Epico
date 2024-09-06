#pragma once

class VulkanAPI: public RenderAPI {
public:
	VulkanAPI(AppContext *context);
	~VulkanAPI() override;

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
	void bind_program(GraphicsProgramHandle handle) override;
	void bind_uniform(LayoutHandle layout, std::span<UniformBind> binds) override;

	void draw(uint32_t vertex_count, uint32_t instance_count) override;
	void draw_instanced(uint32_t index_count, uint32_t instance_count, uint32_t first_instance = 0) override;

	void show_image(TextureHandle handle) override;

	void on_resize(ResizeEventFunction &&resize) override {
		resize_event = resize;
	};

	TextureHandle create_texture() override;
	TextureViewHandle create_texture_view() override;
	SamplerHandle create_sampler() override;
	BufferHandle create_buffer() override;
	ShaderHandle create_shader() override;

	void buffer(BufferHandle handle, BufferType type, size_t size, void *data) override;
	void buffer_sub(BufferHandle handle, size_t offset, size_t size, void *data) override;

	void texture(
		TextureHandle handle,
		ImageDimensions dimensions,
		ImageSamples samples,
		ImageFormat format,
		ImageFlags flags,
		void *data,
		int width, int height
	);

	void sampler(
		SamplerHandle handle,
		SamplerAddressMode u,
		SamplerAddressMode v,
		SamplerAddressMode w
	) override;

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

	void shader(
		ShaderHandle handle,
		ShaderStage shader_type,
		const char *data,
		size_t size,
		const char *entry_point = "main"
	) override;

	void shader(
		ShaderHandle handle,
		ShaderStage shader_type,
		std::span<const char> data,
		const char *entry_point = "main"
	) override { shader(handle, shader_type, data.data(), data.size(), entry_point); }

	RenderLayoutBuilder &create_layout() override;
	GraphicsProgramBuilder &create_graphics_program() override;

	RenderUI *ui() { return &ui_imgui; };

protected:
	void rebuild();
private:
	AppContext *context;
	std::shared_ptr<spdlog::logger> logger;

	VulkanInstance instance;
	VulkanSurface surface;
	VulkanDevice device;
	VulkanSwapchain swapchain;
	VulkanCommandPool command_pool;
	VulkanResourceManager resource_manager;
	VulkanUI ui_imgui;

	ResizeEventFunction resize_event;
};