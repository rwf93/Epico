#pragma once

typedef uint64_t UniformHandle;

enum UniformType {
    UNIFORM_BUFFER,
};

enum UniformState {
    UNIFORM_UNREADY,
    UNIFORM_READY
};

enum UniformStageFlag {
    UNIFORM_STAGE_VERTEX = 1 << 0,
    UNIFORM_STAGE_FRAGMENT = 1 << 1
};

class AbstractUniform {
public:
    virtual ~AbstractUniform() {};
    virtual UniformState get_state() = 0;
    virtual void fini() = 0;
};