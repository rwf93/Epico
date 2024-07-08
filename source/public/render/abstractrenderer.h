#pragma once

#include <public/render/abstractresource.h>
#include <public/render/abstractshader.h>
#include <public/render/abstractuniform.h>
#include <public/render/abstractui.h>

#include <functional>

enum AttachmentType {
	COLOR,
	DEPTH
};

struct SubpassDependency {
	ResourceHandle *attachments;
	AttachmentType *types;
	uint32_t count;
};

class AbstractUI;
class AbstractRenderer {
public:
	virtual ~AbstractRenderer() {};

	virtual void begin() = 0;
	virtual void end() = 0;

	virtual void begin_pass(SubpassDependency *dependencies) = 0;
	virtual void end_pass(SubpassDependency *dependencies) = 0;

	virtual void present() = 0;

	virtual void clear(float r, float g, float b, float a) = 0;
	virtual void viewport(float width, float height, float x = 0, float y = 0) = 0;
	virtual void scissor(uint32_t width, uint32_t height, int32_t x = 0, int32_t y = 0) = 0;

	virtual void bind_buffer(ResourceHandle handle, BindBufferType type) = 0;
	virtual void bind_graphic_shader(ShaderHandle handle) = 0;

	virtual void draw(uint32_t vertex_count, uint32_t index_count) = 0;
	virtual void draw_instanced(uint32_t index_count, uint32_t instance_count, uint32_t index) = 0;
	// Copies the image resource to the surface.
	virtual void show_image(ResourceHandle handle) = 0;

	using ResizeEventFunction = std::function<void(AbstractRenderer*)>;
	virtual void on_resize(ResizeEventFunction &&resize) = 0;

	virtual ResourceHandle create_image() = 0;
	virtual ResourceHandle create_buffer() = 0;

	virtual AbstractGraphicShaderBuilder *create_graphic_shader() = 0;

	virtual void buffer_data(ResourceHandle handle, BufferType type, size_t size, void *data) = 0;
	virtual void buffer_sub_data(ResourceHandle handle, size_t offset, size_t size, void *data) = 0;

	virtual void image_data(
		ResourceHandle handle,
		ImageDimensions dimensions,
		ImageSamples samples,
		ImageFormat format,
		ImageFlags flags,
		void *data,
		int width, int height, int depth = 1
	) = 0;

	virtual AbstractUI *ui() = 0;

};