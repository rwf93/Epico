#pragma once

typedef uint64_t ShaderHandle;

enum ShaderType {
    GRAPHIC,
    COMPUTE
};

enum ShaderState {
    SHADER_READY,
    SHADER_UNREADY,
};

class AbstractShader {
    virtual ShaderType get_type() = 0;
    virtual ShaderState get_state() = 0;
};