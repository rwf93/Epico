#pragma once

enum class ShaderStage;

enum class UniformType {
    BUFFER,
    TEXTURE,
    STORAGE
};

class RenderLayoutBuilder {
public:
    virtual RenderLayoutBuilder &add_uniform(ShaderStage stage, UniformType type) = 0;
    virtual LayoutHandle build() = 0;
};