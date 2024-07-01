#pragma once

class VulkanShaderManager {
public:
    VulkanShaderManager();
    ~VulkanShaderManager();

    void init(FunctorQueue<> &queue);
    void fini();

    ShaderHandle create_graphic_shader();
    ShaderHandle create_compute_shader();

protected:
    ShaderHandle advance_shader_handle() {
        ShaderHandle last_shader_handle = current_shader_handle;
        current_shader_handle++;
        return last_shader_handle;
    }

private:
    std::map<ShaderHandle, AbstractShader*> shaders;
    ShaderHandle current_shader_handle = 0;
};