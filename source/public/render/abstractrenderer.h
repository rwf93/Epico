#pragma once

#include <public/render/abstractresource.h>

class AbstractUI;
class AbstractRenderer {
public:
	virtual ~AbstractRenderer() {};

	virtual void begin() = 0;
	virtual void end() = 0;

	virtual void begin_pass() = 0;
	virtual void end_pass() = 0;

	virtual void clear(float r, float g, float b, float a) = 0;
	virtual void clear_image(ResourceHandle handle) = 0;

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