#pragma once

typedef uint64_t ResourceHandle;

enum class BufferType {
	VERTEX,
	INSTANCE,
	UNIFORM,
	STORAGE
};

enum class BindBufferType {
	VERTEX,
	INSTANCE
};

enum class ImageFormat {
	D32_SFLOAT,

	R8G8B8A8_UNORM,
	R8G8B8A8_SNORM,

	R16G16B16A16_UINT,
	R16G16B16A16_SINT,
	R16G16B16A16_SFLOAT,
};

enum class ImageFlags {
	COLOR_ATTACHMENT = 1 << 0,
	DEPTH_ATTACHMENT = 1 << 1,
	SAMPLED = 1 << 2,
	MIPMAPPED = 1 << 3
};

inline ImageFlags operator | ( ImageFlags lhs, ImageFlags rhs ) {
	return static_cast<ImageFlags>( static_cast<int>(lhs) | static_cast<int>(rhs) );
}

inline bool operator & ( ImageFlags lhs, ImageFlags rhs ) {
	return static_cast<bool>( static_cast<int>(lhs) & static_cast<int>(rhs) );
}

enum class ImageSamples {
	SAMPLE_COUNT_1_BIT,
	SAMPLE_COUNT_2_BIT,
	SAMPLE_COUNT_4_BIT,
	SAMPLE_COUNT_8_BIT,
	SAMPLE_COUNT_16_BIT,
	SAMPLE_COUNT_32_BIT,
	SAMPLE_COUNT_64_BIT,
};

enum class ImageDimensions {
	IMAGE_1D,
	IMAGE_2D,
	IMAGE_3D
};

enum class ResourceState {
	READY,
	UNREADY
};

// Internal structure, expected to be used by the renderer implementation. ResourceHandles are public usage.
class AbstractResource {
public:
	virtual ~AbstractResource() {}
	virtual ResourceState get_state() = 0;
	virtual void fini() = 0; // Only finishes if a resource's state is READY.
};