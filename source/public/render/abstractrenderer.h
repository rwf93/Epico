#pragma once

#include <public/render/abstractresource.h>

class AbstractUI;
class AbstractRenderer {
public:
	virtual ~AbstractRenderer() {};

	virtual void begin() = 0;
	virtual void end() = 0;

	virtual void clear(float r, float g, float b, float a) = 0;

	virtual ResourceHandle create_image() = 0;
	virtual ResourceHandle create_buffer() = 0;

	virtual void buffer_data(ResourceHandle handle, void *data, size_t size, BufferType type) = 0;
	virtual void buffer_sub_data(ResourceHandle handle, void *data, size_t size, size_t offset) = 0;

	virtual AbstractUI *ui() = 0;
};