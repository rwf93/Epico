#pragma once

#include <public/apploader.h>
#include <public/render/renderresource.h>
#include <public/render/renderlayout.h>
#include <public/render/renderprogram.h>
#include <public/render/renderui.h>

#include <functional>
#include <span>
#include <array>
#include <optional>

struct DebugLabel {
	const char *name;
	std::array<float, 4> rgba = { 0.0f, 0.0f, 0.0f, 1.0f };
};

enum class AttachmentType {
	COLOR,
	DEPTH,
	SHADER // Dependency that is used in shaders (i.e textures)
};

struct SubpassAttachment {
	TextureHandle texture;
	TextureViewHandle view;
	AttachmentType type;

	struct ClearValue {
		std::array<float, 4> rgba = { 0.0f, 0.0f, 0.0f, 1.0f };
		float depth = 0.0f;
		uint32_t stencil = 0;
	};

	std::optional<ClearValue> clear = std::nullopt;
};

struct BufferBind {
	BufferHandle buffer = BufferHandle::Invalid;
	size_t offset = 0;
	size_t range = 0;
};

struct TextureBind {
	TextureViewHandle view = TextureViewHandle::Invalid;
	SamplerHandle sampler = SamplerHandle::Invalid;
};

struct UniformBind {
	BufferBind buffer = {};
	TextureBind texture = {};
	UniformType type;
};

struct AppContext;

class RenderUI;
class RenderAPI: public RefCountable {
public:
	virtual ~RenderAPI() = default;

	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void present() = 0;

	virtual void begin_label(DebugLabel marker) = 0;
	virtual void end_label() = 0;

	virtual void begin_pass(std::span<SubpassAttachment> dependencies) = 0;
	virtual void end_pass() = 0;

	virtual void clear(TextureHandle handle, float r, float g, float b, float a) = 0;
	virtual void clear(float r, float g, float b, float a) = 0;

	virtual void viewport(float width, float height, float x = 0, float y = 0) = 0;
	virtual void scissor(uint32_t width, uint32_t height, int32_t x = 0, int32_t y = 0) = 0;

	virtual void bind_buffer(BufferHandle handle, BindBufferType type) = 0;
	virtual void bind_program(GraphicsProgramHandle handle) = 0;
	virtual void bind_uniform(LayoutHandle layout, std::span<UniformBind> binds) = 0;

	virtual void draw(uint32_t vertex_count, uint32_t index_count) = 0;
	virtual void draw_instanced(uint32_t index_count, uint32_t instance_count, uint32_t first_instance = 0) = 0;
	// Copies the texture resource to the surface.
	virtual void show_image(TextureHandle handle) = 0;

	using ResizeEventFunction = std::function<void(RenderAPI*)>;
	virtual void on_resize(ResizeEventFunction &&resize) = 0;

	virtual TextureHandle create_texture() = 0;
	virtual TextureViewHandle create_texture_view() = 0;
	virtual SamplerHandle create_sampler() = 0;
	virtual BufferHandle create_buffer() = 0;
	virtual ShaderHandle create_shader() = 0;

	virtual RenderLayoutBuilder &create_layout() = 0;
	virtual GraphicsProgramBuilder &create_graphics_program() = 0;

	virtual void buffer(BufferHandle handle, BufferType type, size_t size, void *data) = 0;
	virtual void buffer_sub(BufferHandle handle, size_t offset, size_t size, void *data) = 0;

	virtual void texture(
		TextureHandle handle,
		ImageDimensions dimensions,
		ImageSamples samples,
		ImageFormat format,
		ImageFlags flags,
		void *data,
		int width, int height
	) = 0;

	virtual void sampler(SamplerHandle, SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w) = 0;

	virtual void texture_view(
		TextureViewHandle view_handle,
		TextureHandle texture_handle,
		ImageViewDimensions dimensions,
		ImageFormat format,
		int min_level,
		int min_layers,
		int num_levels = 1,
		int num_layers = 1
	) = 0;

	virtual void shader(
		ShaderHandle handle,
		ShaderStage shader_type,
		const char *data,
		size_t size,
		const char *entry_point = "main"
	) = 0;

	virtual void shader(
		ShaderHandle handle,
		ShaderStage shader_type,
		std::span<const char> data,
		const char *entry_point = "main"
	) = 0;

	virtual void *map(BufferHandle handle) = 0;
	virtual void unmap(BufferHandle handle) = 0;

	virtual RenderUI *ui() = 0;
};
