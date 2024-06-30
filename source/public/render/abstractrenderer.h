#pragma once

#include <public/render/abstractresource.h>

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
	virtual void end_pass() = 0;

	virtual void present(ResourceHandle handle) = 0;
	virtual void clear(ResourceHandle handle, float r, float g, float b, float a) = 0;

	virtual void viewport(float width, float height, float x = 0, float y = 0) = 0;
	virtual void scissor(uint32_t width, uint32_t height, int32_t x = 0, int32_t y = 0) = 0;

	virtual ResourceHandle create_image() = 0;
	virtual ResourceHandle create_buffer() = 0;

	virtual void buffer_data(ResourceHandle handle, BufferType type, size_t size, void *data) = 0;
	virtual void buffer_sub_data(ResourceHandle handle, size_t offset, size_t size, void *data) = 0;

	virtual void image_data(
		ResourceHandle handle,
		ImageDimensions dimensions,
		ImageSamples samples,
		ImageFormat format,
		void *data,
		bool mipmapped,
		int width, int height, int depth = 1
	) = 0;

	virtual AbstractUI *ui() = 0;
};