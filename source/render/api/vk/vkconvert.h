#pragma once

namespace convert {

#define CONVERTER_HEADER(name, A, B) \
	B name (A type)

// A represents the type you are converting to, B represents the type you're converting from.
// Invariant is the result of when there is no suitable conversion to A.
// Varadict arguments is the conversion table.
// Check vkconvert.cpp for an example.
#define CONVERTER(name, A, B, invariant, ...)       	\
	inline B name (A type) {                  		\
		static std::map<A, B> name##_table = {      	\
			__VA_ARGS__                             	\
		};                                          	\
		if(name##_table.contains(type))					\
			return name##_table[type];					\
		assert("Unable to find a suitable conversion"); \
		return invariant;                           	\
	}

#pragma optimize("", off)

CONVERTER(convert_image_format, ImageFormat, VkFormat, VK_FORMAT_UNDEFINED,
	{ ImageFormat::D32_SFLOAT, VK_FORMAT_D32_SFLOAT },
	{ ImageFormat::R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM },
	{ ImageFormat::R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_SNORM },
	{ ImageFormat::R16G16B16A16_UINT, VK_FORMAT_R16G16B16A16_UINT },
	{ ImageFormat::R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SINT },
	{ ImageFormat::R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT },
);

CONVERTER(convert_sample_bits, ImageSamples, VkSampleCountFlagBits, VK_SAMPLE_COUNT_1_BIT,
	{ ImageSamples::SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_1_BIT },
	{ ImageSamples::SAMPLE_COUNT_2_BIT, VK_SAMPLE_COUNT_2_BIT },
	{ ImageSamples::SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_4_BIT },
	{ ImageSamples::SAMPLE_COUNT_8_BIT, VK_SAMPLE_COUNT_8_BIT },
	{ ImageSamples::SAMPLE_COUNT_16_BIT, VK_SAMPLE_COUNT_16_BIT },
	{ ImageSamples::SAMPLE_COUNT_32_BIT, VK_SAMPLE_COUNT_32_BIT },
	{ ImageSamples::SAMPLE_COUNT_64_BIT, VK_SAMPLE_COUNT_64_BIT }
);

inline VkBufferUsageFlagBits convert_buffer_type(BufferType type) {
	uint64_t bits = 0;
	if(type & BufferType::VERTEX)
		bits |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	if(type & BufferType::INSTANCE)
		bits |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	if(type & BufferType::UNIFORM)
		bits |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	if(type & BufferType::STORAGE)
		bits |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	return static_cast<VkBufferUsageFlagBits>(bits);
}

CONVERTER(convert_image_dimensions, ImageDimensions, VkImageType, VK_IMAGE_TYPE_1D,
	{ ImageDimensions::IMAGE_1D, VK_IMAGE_TYPE_1D },
	{ ImageDimensions::IMAGE_2D, VK_IMAGE_TYPE_2D },
	{ ImageDimensions::IMAGE_3D, VK_IMAGE_TYPE_3D },
);

CONVERTER(convert_image_view_dimensions, ImageViewDimensions, VkImageViewType, VK_IMAGE_VIEW_TYPE_1D,
	{ ImageViewDimensions::IMAGE_1D, VK_IMAGE_VIEW_TYPE_1D },
	{ ImageViewDimensions::IMAGE_2D, VK_IMAGE_VIEW_TYPE_2D },
	{ ImageViewDimensions::IMAGE_3D, VK_IMAGE_VIEW_TYPE_3D },
);

CONVERTER(convert_attribute_format, AttributeType, VkFormat, VK_FORMAT_UNDEFINED,
	{ AttributeType::VEC4D_SIGNED, VK_FORMAT_R32G32B32A32_SFLOAT },
	{ AttributeType::VEC3D_SIGNED, VK_FORMAT_R32G32B32_SFLOAT },
	{ AttributeType::VEC2D_SIGNED, VK_FORMAT_R32G32_SFLOAT },
	{ AttributeType::VEC1D_SIGNED, VK_FORMAT_R32_SFLOAT },
);

CONVERTER(convert_binding_rate, BindingRate, VkVertexInputRate, VK_VERTEX_INPUT_RATE_VERTEX,
	{ BindingRate::VERTEX, VK_VERTEX_INPUT_RATE_VERTEX },
	{ BindingRate::INDEX, VK_VERTEX_INPUT_RATE_INSTANCE },
);

CONVERTER(convert_primitive_type, ShaderPrimitive, VkPrimitiveTopology, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	{ ShaderPrimitive::TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST },
	{ ShaderPrimitive::TRIANGLE_STRIP, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP },
	{ ShaderPrimitive::POINT_LIST, VK_PRIMITIVE_TOPOLOGY_POINT_LIST },
);

CONVERTER(convert_polygon_mode, ShaderPolygonMode, VkPolygonMode, VK_POLYGON_MODE_FILL,
	{ ShaderPolygonMode::FILL, VK_POLYGON_MODE_FILL },
	{ ShaderPolygonMode::LINE, VK_POLYGON_MODE_LINE },
	{ ShaderPolygonMode::POINT, VK_POLYGON_MODE_POINT },
);

CONVERTER(convert_compare_op, ShaderCompareOp, VkCompareOp, VK_COMPARE_OP_NEVER,
	{ ShaderCompareOp::NEVER, VK_COMPARE_OP_NEVER },
	{ ShaderCompareOp::ALWAYS, VK_COMPARE_OP_ALWAYS },
	{ ShaderCompareOp::LESS_OR_EQUAL, VK_COMPARE_OP_LESS_OR_EQUAL },
	{ ShaderCompareOp::GREATER_OR_EQUAL, VK_COMPARE_OP_GREATER_OR_EQUAL },
);

CONVERTER(convert_uniform_type, UniformType, VkDescriptorType, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	{ UniformType::BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
	{ UniformType::TEXTURE, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER },
	{ UniformType::STORAGE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
);

#pragma optimize("", on)

}
