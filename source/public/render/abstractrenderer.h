#pragma once

#include <public/render/abstractresource.h>

class AbstractUI;
class AbstractRenderer {
public:
	virtual ~AbstractRenderer() {};

	virtual void begin() = 0;
	virtual void end() = 0;

	virtual void clear(float r, float g, float b, float a) = 0;

	virtual ResourceHandle create_image(
		int width,
		int height,
		int depth = 1,
		ImageDimensions dimensions = ImageDimensions::IMAGE_2D,
		ImageFormat format = ImageFormat::R16G16B16A16_UINT,
		ImageSample samples = ImageSample::SAMPLE_COUNT_16_BIT
	) = 0;

	virtual AbstractUI *ui() = 0;
};