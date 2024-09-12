#pragma once

#include <span>

enum class BindingRate {
	VERTEX,
	INDEX
};

enum class AttributeType {
	MAT4F_SIGNED,
	VEC4F_SIGNED,
	VEC3F_SIGNED,
	VEC2F_SIGNED,
	VEC1F_SIGNED
};

enum class ShaderStage {
	VERTEX = 1 << 0,
	FRAGMENT = 1 << 1,
	GEOMETRY = 1 << 2,
};

inline ShaderStage operator | ( ShaderStage lhs, ShaderStage rhs ) {
	return static_cast<ShaderStage>( static_cast<int>(lhs) | static_cast<int>(rhs) );
}

inline bool operator & ( ShaderStage lhs, ShaderStage rhs ) {
	return static_cast<bool>( static_cast<int>(lhs) & static_cast<int>(rhs) );
}

enum class PrimitiveMode {
	TRIANGLE_LIST,
	TRIANGLE_STRIP,
	POINT_LIST
};

enum class PolygonMode {
	FILL,
	LINE,
	POINT
};

enum class CompareOp {
	NEVER,
	EQUAL,
	LESS,
	ALWAYS,
	LESS_OR_EQUAL,
	GREATER_OR_EQUAL,
	GREATER
};

enum class CullFace {
	NONE,
	FRONT,
	BACK,
};

enum class FrontFace {
	CLOCKWISE,
	COUNTER_CLOCKWISE
};

class GraphicsProgramBuilder {
public:
	virtual ~GraphicsProgramBuilder() {}

	virtual GraphicsProgramHandle build() = 0;

	// Use these if you want to call add_* functions after a previous .build()
	virtual GraphicsProgramBuilder &clear_stages() = 0;
	virtual GraphicsProgramBuilder &clear_bindings() = 0;
	virtual GraphicsProgramBuilder &clear_attributes() = 0;
	virtual GraphicsProgramBuilder &clear_attachments() = 0;

	virtual GraphicsProgramBuilder &set_primitive(PrimitiveMode type) = 0;
	virtual GraphicsProgramBuilder &set_polygon_mode(PolygonMode mode) = 0;
	virtual GraphicsProgramBuilder &set_depth_format(ImageFormat format) = 0;
	virtual GraphicsProgramBuilder &set_depth_test(
		bool test_enable,
		bool write_enable,
		CompareOp compare
	) = 0;

	virtual GraphicsProgramBuilder &set_cull_face(CullFace face) = 0;
	virtual GraphicsProgramBuilder &set_front_face(FrontFace face) = 0;

	virtual GraphicsProgramBuilder &add_binding(
		uint32_t size,
		BindingRate rate,
		uint32_t binding = 0
	) = 0;

	virtual GraphicsProgramBuilder &add_attribute(
		uint32_t offset,
		AttributeType type,
		uint32_t binding = 0
	) = 0;

	virtual GraphicsProgramBuilder &add_attachment(ImageFormat format) = 0;

	virtual GraphicsProgramBuilder &add_stage(
		ShaderHandle shader
	) = 0;

	virtual GraphicsProgramBuilder &set_layout(LayoutHandle layout) = 0;
};