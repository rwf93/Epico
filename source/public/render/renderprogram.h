#pragma once

enum class BindingRate {
	VERTEX,
	INDEX
};

enum class AttributeType {
	VEC4D_SIGNED,
	VEC3D_SIGNED,
	VEC2D_SIGNED,
	VEC1D_SIGNED
};

enum class ShaderStage {
	VERTEX = 1 << 0,
	FRAGMENT = 1 << 1
};

inline ShaderStage operator | ( ShaderStage lhs, ShaderStage rhs ) {
	return static_cast<ShaderStage>( static_cast<int>(lhs) | static_cast<int>(rhs) );
}

inline bool operator & ( ShaderStage lhs, ShaderStage rhs ) {
	return static_cast<bool>( static_cast<int>(lhs) & static_cast<int>(rhs) );
}

enum class ShaderPrimitive {
	TRIANGLE_LIST,
	TRIANGLE_STRIP,
	POINT_LIST
};

enum class ShaderPolygonMode {
	FILL,
	LINE,
	POINT
};

enum class ShaderCompareOp {
	NEVER,
	LESS,
	ALWAYS,
	LESS_OR_EQUAL,
	GREATER_OR_EQUAL,
};

class GraphicsProgramBuilder {
public:
	virtual ~GraphicsProgramBuilder() {}
	virtual GraphicsProgramHandle build() = 0;

	virtual GraphicsProgramBuilder *set_primitive(ShaderPrimitive type) = 0;
	virtual GraphicsProgramBuilder *set_polygon_mode(ShaderPolygonMode mode) = 0;
	virtual GraphicsProgramBuilder *set_depth_format(ImageFormat format) = 0;
	virtual GraphicsProgramBuilder *set_depth_test(bool write_enable, ShaderCompareOp compare) = 0;

	virtual GraphicsProgramBuilder *add_binding(
		uint32_t size,
		BindingRate rate,
		uint32_t binding = 0
	) = 0;

	virtual GraphicsProgramBuilder *add_attribute(
		uint32_t offset,
		AttributeType type,
		uint32_t binding = 0
	) = 0;

	virtual GraphicsProgramBuilder *add_attachment(ImageFormat format) = 0;

	virtual GraphicsProgramBuilder *add_stage(
		ShaderStage stage,
		const char *data,
		size_t size
	) = 0;

	virtual GraphicsProgramBuilder *set_layout(LayoutHandle layout) = 0;
};