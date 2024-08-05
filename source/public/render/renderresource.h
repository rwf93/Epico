#pragma once

enum class TextureHandle: 			size_t { Invalid = 0 };
enum class TextureViewHandle: 		size_t { Invalid = 0 };
enum class SamplerHandle:			size_t { Invalid = 0 };
enum class BufferHandle: 			size_t { Invalid = 0 };
enum class LayoutHandle: 			size_t { Invalid = 0 };
enum class GraphicsProgramHandle: 	size_t { Invalid = 0 };

enum class BufferType {
	VERTEX  = 1 << 0,
	INSTANCE = 1 << 1,
	UNIFORM = 1 << 2,
	STORAGE = 1 << 3
};

inline bool operator & ( BufferType lhs, BufferType rhs ) {
	return static_cast<bool>( static_cast<int>(lhs) & static_cast<int>(rhs) );
}

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

enum class ImageViewDimensions {
	IMAGE_1D,
	IMAGE_2D,
	IMAGE_3D
};

enum class SamplerAddressMode {
	REPEAT,
	MIRRORED_REPEAT,
	CLAMP_EDGE,
	CLAMP_BORDER
};

enum class ResourceState {
	READY,
	UNREADY
};

// Internal structure, expected to be used by the API implementation. ResourceHandles are public usage.
class RenderResource {
public:
	virtual ~RenderResource() {}
	virtual ResourceState get_state() = 0;
	virtual void fini() = 0; // Only finishes if a resource's state is READY.
};