#pragma once

typedef uint64_t ShaderHandle;

enum BindingRate {
	RATE_VERTEX,
	RATE_INDEX
};

enum AttributeType {
	VEC4D_SIGNED,
	VEC3D_SIGNED,
	VEC2D_SIGNED,
	VEC1D_SIGNED
};

enum ShaderState {
	SHADER_READY,
	SHADER_UNREADY,
};

enum ShaderStage {
	STAGE_VERTEX = 1 << 0,
	STAGE_FRAGMENT = 1 << 1
};

enum ShaderPrimitive {
	TRIANGLE_LIST,
	TRIANGLE_STRIP,
	POINT_LIST
};

enum ShaderPolygonMode {
	MODE_FILL,
	MODE_LINE,
	MODE_POINT
};

enum ShaderCompareOp {
	COMPARE_NEVER,
	COMPARE_ALWAYS,
	COMPARE_LESS_OR_EQUAL,
	COMPARE_GREATER_OR_EQUAL,
};

class AbstractShader {
public:
	virtual ~AbstractShader() {}
	virtual ShaderState get_state() = 0;
	virtual void fini() = 0;
};

class AbstractShaderBuilder {
public:
	virtual ~AbstractShaderBuilder() {}
	virtual ShaderHandle build() = 0;
};

class AbstractGraphicShaderBuilder: public AbstractShaderBuilder {
public:
	virtual AbstractGraphicShaderBuilder *set_primitive(ShaderPrimitive type) = 0;
	virtual AbstractGraphicShaderBuilder *set_polygon_mode(ShaderPolygonMode mode) = 0;
	virtual AbstractGraphicShaderBuilder *set_depth_format(ImageFormat format) = 0;
	virtual AbstractGraphicShaderBuilder *set_depth_test(bool write_enable, ShaderCompareOp compare) = 0;

	virtual AbstractGraphicShaderBuilder *add_binding(
		uint32_t binding,
		uint32_t size,
		BindingRate rate
	) = 0;

	virtual AbstractGraphicShaderBuilder *add_attribute(
		uint32_t location,
		uint32_t binding,
		uint32_t offset,
		AttributeType type
	) = 0;

	virtual AbstractGraphicShaderBuilder *add_attachment(ImageFormat format) = 0;

	virtual AbstractGraphicShaderBuilder *add_uniform(
		uint32_t set,
		uint32_t binding,
		UniformType uniform,
		ShaderStage stage
	) = 0;

	virtual AbstractGraphicShaderBuilder *add_stage(
		ShaderStage stage,
		const char *data,
		size_t size
	) = 0;
};