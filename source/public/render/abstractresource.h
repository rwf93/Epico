#pragma once

typedef uint32_t ResourceHandle;

enum ResourceType {
    IMAGE,
    BUFFER
};

enum ImageFormat {
    R16G16B16A16_UINT,
    R16G16B16A16_SINT,
    R16G16B16A16_SFLOAT,
};

enum ImageSample {
    SAMPLE_COUNT_1_BIT,
    SAMPLE_COUNT_2_BIT,
    SAMPLE_COUNT_4_BIT,
    SAMPLE_COUNT_8_BIT,
    SAMPLE_COUNT_16_BIT,
    SAMPLE_COUNT_32_BIT,
    SAMPLE_COUNT_64_BIT,
};

enum ImageDimensions {
    IMAGE_2D,
    IMAGE_3D
};

class AbstractResource {
public:
    virtual ~AbstractResource() {}
    virtual ResourceType get_type() = 0;
};