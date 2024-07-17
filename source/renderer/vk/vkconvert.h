#pragma once

namespace convert {

#define CONVERTER_HEADER(name, A, B) \
	B name (A type)

// A represents the type you are converting to, B represents the type you're converting from.
// Invariant is the result of when there is no suitable conversion to A.
// Varadict arguments is the conversion table.
// Check vkconvert.cpp for an example.
#define CONVERTER(name, A, B, invariant, ...)       	\
	B convert :: name (A type) {                  		\
		static std::map<A, B> name##_table = {      	\
			__VA_ARGS__                             	\
		};                                          	\
		if(name##_table.contains(type))					\
			return name##_table[type];					\
		assert("Unable to find a suitable conversion"); \
		return invariant;                           	\
	}

CONVERTER_HEADER(convert_image_format, ImageFormat, VkFormat);
CONVERTER_HEADER(convert_sample_bits, ImageSamples, VkSampleCountFlagBits);
CONVERTER_HEADER(convert_buffer_type, BufferType, VkBufferUsageFlagBits);
CONVERTER_HEADER(convert_image_dimensions, ImageDimensions, VkImageType);
CONVERTER_HEADER(convert_image_view_dimensions, ImageDimensions, VkImageViewType);
CONVERTER_HEADER(convert_attribute_format, AttributeType, VkFormat);
CONVERTER_HEADER(convert_binding_rate, BindingRate, VkVertexInputRate);
CONVERTER_HEADER(convert_primitive_type, ShaderPrimitive, VkPrimitiveTopology);
CONVERTER_HEADER(convert_polygon_mode, ShaderPolygonMode, VkPolygonMode);
CONVERTER_HEADER(convert_uniform_type, UniformType, VkDescriptorType);
CONVERTER_HEADER(convert_compare_op, ShaderCompareOp, VkCompareOp);

}
