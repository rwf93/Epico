#pragma once

typedef uint64_t LayoutHandle;

enum class ShaderStage;

enum LayoutState {
    LAYOUT_UNREADY,
    LAYOUT_READY
};

class AbstractLayout {
public:
    virtual ~AbstractLayout() {}
    virtual LayoutState get_state() = 0;
    virtual void fini() = 0;
};

class AbstractLayoutBuilder {
public:
    virtual AbstractLayoutBuilder *add_uniform(ShaderStage stage) = 0;
    virtual LayoutHandle build() = 0;
};