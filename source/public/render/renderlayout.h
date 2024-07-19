#pragma once

typedef uint64_t LayoutHandle;

enum class ShaderStage;

enum LayoutState {
    LAYOUT_UNREADY,
    LAYOUT_READY
};

class RenderLayout {
public:
    virtual ~RenderLayout() {}
    virtual LayoutState get_state() = 0;
    virtual void fini() = 0;
};

class RenderLayoutBuilder {
public:
    virtual RenderLayoutBuilder *add_uniform(ShaderStage stage) = 0;
    virtual LayoutHandle build() = 0;
};