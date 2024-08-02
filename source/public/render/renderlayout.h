#pragma once

enum class LayoutHandle { Invalid = 0 };

enum class ShaderStage;

enum LayoutState {
    LAYOUT_UNREADY,
    LAYOUT_READY
};

enum class UniformType {
    BUFFER,
    TEXTURE,
    STORAGE
};

class RenderLayout {
public:
    virtual ~RenderLayout() {}
    virtual LayoutState get_state() = 0;
    virtual void fini() = 0;
};

class RenderLayoutBuilder {
public:
    virtual RenderLayoutBuilder *add_uniform(ShaderStage stage, UniformType type) = 0;
    virtual LayoutHandle build() = 0;
};