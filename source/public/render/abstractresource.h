#pragma once

typedef uint64_t ResourceHandle;

enum BufferType {
    BUFFER_VERTEX,
    BUFFER_INDEX,
    BUFFER_STORAGE
};

enum BindBufferType {
    BIND_VERTEX,
    BIND_INDEX
};

enum ImageFormat {
    R8G8B8A8_UNORM,
    R8G8B8A8_SNORM,

    R16G16B16A16_UINT,
    R16G16B16A16_SINT,
    R16G16B16A16_SFLOAT,
};

enum ImageSamples {
    SAMPLE_COUNT_1_BIT,
    SAMPLE_COUNT_2_BIT,
    SAMPLE_COUNT_4_BIT,
    SAMPLE_COUNT_8_BIT,
    SAMPLE_COUNT_16_BIT,
    SAMPLE_COUNT_32_BIT,
    SAMPLE_COUNT_64_BIT,
};

enum ImageDimensions {
    IMAGE_1D,
    IMAGE_2D,
    IMAGE_3D
};

enum ResourceState {
    RESOURCE_READY,
    RESOURCE_UNREADY
};

// Internal structure, expected to be used by the renderer implementation. ResourceHandles are public usage.
class AbstractResource {
public:
    virtual ~AbstractResource() {}
    virtual ResourceState get_state() = 0;
    virtual void fini() = 0; // Only finishes if a resource's state is READY.
};