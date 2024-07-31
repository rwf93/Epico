#pragma once

#include <public/render/renderresource.h>
#include <public/render/renderlayout.h>
#include <public/render/renderprogram.h>
#include <public/render/renderui.h>

#include <functional>

enum class AttachmentType {
	COLOR,
	DEPTH
};

struct SubpassAttachment {
	TextureHandle texture;
	TextureViewHandle view;
	AttachmentType type;

	struct ClearValue {
		float r, g, b, a;
		float depth;
		uint32_t stencil;
	} clear;
};

struct SubpassDependencyInfo {
	SubpassAttachment *attachments;
	uint32_t count;
};

struct AppContext;

class RenderUI;
class RenderAPI {
public:
	virtual ~RenderAPI() {};

	virtual void init(AppContext *context) = 0;

	virtual void begin() = 0;
	virtual void end() = 0;

	virtual void begin_pass(SubpassDependencyInfo *dependencies) = 0;
	virtual void end_pass(SubpassDependencyInfo *dependencies) = 0;

	virtual void present() = 0;

	virtual void clear(TextureHandle handle, float r, float g, float b, float a) = 0;
	virtual void clear(float r, float g, float b, float a) = 0;

	virtual void viewport(float width, float height, float x = 0, float y = 0) = 0;
	virtual void scissor(uint32_t width, uint32_t height, int32_t x = 0, int32_t y = 0) = 0;

	virtual void bind_buffer(BufferHandle handle, BindBufferType type) = 0;
	virtual void bind_shader(GraphicsProgramHandle handle) = 0;
	virtual void bind_uniform(GraphicsProgramHandle shader, BufferHandle handle, size_t offset, size_t range) = 0;

	virtual void draw(uint32_t vertex_count, uint32_t index_count) = 0;
	virtual void draw_instanced(uint32_t index_count, uint32_t instance_count, uint32_t index) = 0;
	// Copies the texture resource to the surface.
	virtual void show_image(TextureHandle handle) = 0;

	using ResizeEventFunction = std::function<void(RenderAPI*)>;
	virtual void on_resize(ResizeEventFunction &&resize) = 0;

	virtual TextureHandle create_texture() = 0;
	virtual TextureViewHandle create_texture_view() = 0;
	virtual BufferHandle create_buffer() = 0;

	virtual RenderLayoutBuilder *create_layout() = 0;
	virtual RenderGraphicProgramBuilder *create_graphic_program() = 0;

	virtual void buffer_data(BufferHandle handle, BufferType type, size_t size, void *data) = 0;
	virtual void buffer_sub_data(BufferHandle handle, size_t offset, size_t size, void *data) = 0;

	virtual void texture_data(
		TextureHandle handle,
		ImageDimensions dimensions,
		ImageSamples samples,
		ImageFormat format,
		ImageFlags flags,
		void *data,
		int width, int height, int depth = 1
	) = 0;

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

	virtual RenderUI *ui() = 0;
};