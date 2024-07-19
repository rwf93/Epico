#pragma once

enum class GraphicsProgramHandle: size_t { Invalid = 0 };

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

enum class ShaderState {
	READY,
	UNREADY,
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
	ALWAYS,
	LESS_OR_EQUAL,
	GREATER_OR_EQUAL,
};

class RenderProgram {
public:
	virtual ~RenderProgram() {}
	virtual ShaderState get_state() = 0;
	virtual void fini() = 0;
};

class RenderGraphicProgramBuilder {
public:
	virtual ~RenderGraphicProgramBuilder() {}
	virtual GraphicsProgramHandle build() = 0;

	virtual RenderGraphicProgramBuilder *set_primitive(ShaderPrimitive type) = 0;
	virtual RenderGraphicProgramBuilder *set_polygon_mode(ShaderPolygonMode mode) = 0;
	virtual RenderGraphicProgramBuilder *set_depth_format(ImageFormat format) = 0;
	virtual RenderGraphicProgramBuilder *set_depth_test(bool write_enable, ShaderCompareOp compare) = 0;

	virtual RenderGraphicProgramBuilder *add_binding(
		uint32_t binding,
		uint32_t size,
		BindingRate rate
	) = 0;

	virtual RenderGraphicProgramBuilder *add_attribute(
		uint32_t location,
		uint32_t binding,
		uint32_t offset,
		AttributeType type
	) = 0;

	virtual RenderGraphicProgramBuilder *add_attachment(ImageFormat format) = 0;

	virtual RenderGraphicProgramBuilder *add_stage(
		ShaderStage stage,
		const char *data,
		size_t size
	) = 0;

	virtual RenderGraphicProgramBuilder *add_layout(LayoutHandle layout) = 0;
};