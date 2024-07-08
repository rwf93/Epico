#pragma once

typedef uint64_t UniformHandle;

enum UniformType {
    UNIFORM_BUFFER,
};

enum UniformState {
    UNIFORM_UNREADY,
    UNIFORM_READY
};

class AbstractUniform {
public:
    virtual ~AbstractUniform() {};
    virtual UniformState get_state() = 0;
    virtual void fini() = 0;
};