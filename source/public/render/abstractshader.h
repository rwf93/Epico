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
    STAGE_VERTEX,
    STAGE_FRAGMENT
};

class AbstractShader {
public:
    virtual ~AbstractShader() {}
    virtual ShaderState get_state() = 0;
    virtual ShaderHandle init() = 0;
    virtual void fini() = 0;
};

class AbstractGraphicShader: public AbstractShader {
public:
    virtual AbstractGraphicShader *add_binding(
        uint32_t binding,
        uint32_t size,
        BindingRate rate
    ) = 0;

    virtual AbstractGraphicShader *add_attribute(
        uint32_t location,
        uint32_t binding,
        uint32_t offset,
        AttributeType type
    ) = 0;

    virtual AbstractGraphicShader *add_stage(
        ShaderStage stage,
        const char *data,
        size_t size
    ) = 0;
};

class AbstractComputeShader: public AbstractShader {

};